#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>

#include <curl/curl.h>
#include <zip.h>
#include <jansson.h>

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#endif
#include <dlfcn.h>
#include <dirent.h>

#include "plugin.hpp"


namespace rack {

std::list<Plugin*> gPlugins;

static const std::string apiUrl = "http://localhost:8081";
static std::string token;
static bool isDownloading = false;
static float downloadProgress = 0.0;
static std::string downloadName;


Plugin::~Plugin() {
	for (Model *model : models) {
		delete model;
	}
}


static int loadPlugin(std::string slug) {
	#if ARCH_LIN
		std::string path = "./plugins/" + slug + "/plugin.so";
	#elif ARCH_WIN
		std::string path = "./plugins/" + slug + "/plugin.dll";
	#elif ARCH_MAC
		std::string path = "./plugins/" + slug + "/plugin.dylib";
	#endif

	// Load dynamic/shared library
	#if ARCH_WIN
		HINSTANCE handle = LoadLibrary(path.c_str());
		if (!handle) {
			fprintf(stderr, "Failed to load library %s\n", path.c_str());
			return -1;
		}
	#elif ARCH_LIN || ARCH_MAC
		void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
		if (!handle) {
			fprintf(stderr, "Failed to load library %s: %s\n", path.c_str(), dlerror());
			return -1;
		}
	#endif

	// Call plugin init() function
	typedef Plugin *(*InitCallback)();
	InitCallback initCallback;
	#if ARCH_WIN
		initCallback = (InitCallback) GetProcAddress(handle, "init");
	#elif ARCH_LIN || ARCH_MAC
		initCallback = (InitCallback) dlsym(handle, "init");
	#endif
	if (!initCallback) {
		fprintf(stderr, "Failed to read init() symbol in %s\n", path.c_str());
		return -2;
	}

	// Add plugin to map
	Plugin *plugin = initCallback();
	if (!plugin) {
		fprintf(stderr, "Library %s did not return a plugin\n", path.c_str());
		return -3;
	}
	gPlugins.push_back(plugin);
	fprintf(stderr, "Loaded plugin %s\n", path.c_str());
	return 0;
}

void pluginInit() {
	curl_global_init(CURL_GLOBAL_NOTHING);

	// Load core
	// This function is defined in core.cpp
	Plugin *corePlugin = init();
	gPlugins.push_back(corePlugin);

	// Search for plugin libraries
	DIR *dir = opendir("plugins");
	if (dir) {
		struct dirent *d;
		while ((d = readdir(dir))) {
			if (d->d_name[0] == '.')
				continue;
			loadPlugin(d->d_name);
		}
		closedir(dir);
	}
}

void pluginDestroy() {
	for (Plugin *plugin : gPlugins) {
		// TODO free shared library handle with `dlclose` or `FreeLibrary`
		delete plugin;
	}
	gPlugins.clear();
	curl_global_cleanup();
}

////////////////////
// CURL and libzip helpers
////////////////////

static size_t write_file_callback(void *data, size_t size, size_t nmemb, void *p) {
	int fd = *((int*)p);
	ssize_t len = write(fd, data, size*nmemb);
	return len;
}

static int progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (dltotal == 0.0)
		return 0;
	float progress = dlnow / dltotal;
	downloadProgress = progress;
	return 0;
}

static CURLcode download_file(int fd, const char *url) {
	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fd);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

static size_t write_string_callback(void *data, size_t size, size_t nmemb, void *p) {
	std::string &text = *((std::string*)p);
	char *dataStr = (char*) data;
	size_t len = size * nmemb;
	text.append(dataStr, len);
	return len;
}

static void extract_zip(const char *dir, int zipfd) {
	int err = 0;
	zip_t *za = zip_fdopen(zipfd, 0, &err);
	if (!za) return;
	if (err) goto cleanup;

	for (int i = 0; i < zip_get_num_entries(za, 0); i++) {
		zip_stat_t zs;
		err = zip_stat_index(za, i, 0, &zs);
		if (err) goto cleanup;
		int nameLen = strlen(zs.name);

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", dir, zs.name);

		if (zs.name[nameLen - 1] == '/') {
			err = mkdir(path, 0755);
			if (err) goto cleanup;
		}
		else {
			zip_file_t *zf = zip_fopen_index(za, i, 0);
			if (!zf) goto cleanup;

			int out = open(path, O_RDWR | O_TRUNC | O_CREAT, 0644);
			assert(out != -1);

			while (1) {
				char buffer[4096];
				int len = zip_fread(zf, buffer, sizeof(buffer));
				if (len <= 0)
					break;
				write(out, buffer, len);
			}

			err = zip_fclose(zf);
			assert(!err);
			close(out);
		}
	}

cleanup:
	zip_close(za);
}

////////////////////
// plugin manager
////////////////////

void pluginOpenBrowser(std::string url) {
	// shell injection is possible, so make sure the URL is trusted
#if ARCH_LIN
	std::string command = "xdg-open " + url;
	system(command.c_str());
#endif
#if ARCH_MAC
	std::string command = "open " + url;
	system(command.c_str());
#endif
#if ARCH_WIN
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}

void pluginLogIn(std::string email, std::string password) {
	CURL *curl = curl_easy_init();
	assert(curl);

	std::string postFields = "email=" + email + "&password=" + password;
	std::string url = apiUrl + "/token";
	std::string resText;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resText);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.size());
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res == CURLE_OK) {
		// Parse JSON response
		json_error_t error;
		json_t *rootJ = json_loads(resText.c_str(), 0, &error);
		if (rootJ) {
			json_t *tokenJ = json_object_get(rootJ, "token");
			if (tokenJ) {
				// Set the token, which logs the user in
				token = json_string_value(tokenJ);
			}
			json_decref(rootJ);
		}
	}
}

void pluginLogOut() {
	token = "";
}

static void pluginRefreshPlugin(json_t *pluginJ) {
	json_t *slugJ = json_object_get(pluginJ, "slug");
	if (!slugJ) return;
	std::string slug = json_string_value(slugJ);

	json_t *nameJ = json_object_get(pluginJ, "name");
	if (!nameJ) return;
	std::string name = json_string_value(nameJ);

	json_t *urlJ = json_object_get(pluginJ, "download");
	if (!urlJ) return;
	std::string url = json_string_value(urlJ);

	// Find slug in plugins list
	for (Plugin *p : gPlugins) {
		if (p->slug == slug) {
			return;
		}
	}

	// If plugin is not loaded, download the zip file to /plugins
	fprintf(stderr, "Downloading %s from %s\n", name.c_str(), url.c_str());
	downloadName = name;
	downloadProgress = 0.0;

	const char *dir = "plugins";
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "%s/%s.zip", dir, slug.c_str());
	int zip = open(path, O_RDWR | O_TRUNC | O_CREAT, 0644);
	// Download zip
	download_file(zip, url.c_str());
	// Unzip file
	lseek(zip, 0, SEEK_SET);
	extract_zip(dir, zip);
	// Close file
	close(zip);
	downloadName = "";

	// Load plugin
}

void pluginRefresh() {
	if (token.empty())
		return;

	isDownloading = true;
	downloadProgress = 0.0;
	downloadName = "";

	// Get plugin list from /plugin
	CURL *curl = curl_easy_init();
	assert(curl);

	std::string url = apiUrl + "/plugins?token=" + token;
	std::string resText;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resText);
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res == CURLE_OK) {
		// Parse JSON response
		json_error_t error;
		json_t *rootJ = json_loads(resText.c_str(), 0, &error);
		if (rootJ) {
			json_t *pluginsJ = json_object_get(rootJ, "plugins");
			if (pluginsJ) {
				// Iterate through each plugin object
				size_t index;
				json_t *pluginJ;
				json_array_foreach(pluginsJ, index, pluginJ) {
					pluginRefreshPlugin(pluginJ);
				}
			}
			json_decref(rootJ);
		}
	}

	isDownloading = false;
}

void pluginCancelDownload() {
	// TODO
}

bool pluginIsLoggedIn() {
	return token != "";
}

bool pluginIsDownloading() {
	return isDownloading;
}

float pluginGetDownloadProgress() {
	return downloadProgress;
}

std::string pluginGetDownloadName() {
	return downloadName;
}


} // namespace rack
