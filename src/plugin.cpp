#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(WINDOWS)
	#include <windows.h>
#elif defined(LINUX) || defined(APPLE)
	#include <dlfcn.h>
	#include <glob.h>
#endif

#include <curl/curl.h>
#include <zip.h>
#include <jansson.h>

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


static int loadPlugin(const char *path) {
	// Load dynamic/shared library
	#if defined(WINDOWS)
		HINSTANCE handle = LoadLibrary(path);
		if (!handle) {
			fprintf(stderr, "Failed to load library %s\n", path);
			return -1;
		}
	#elif defined(LINUX) || defined(APPLE)
		char ppath[1024];
		snprintf(ppath, sizeof(ppath), "./%s", path);
		void *handle = dlopen(ppath, RTLD_NOW | RTLD_GLOBAL);
		if (!handle) {
			fprintf(stderr, "Failed to load library %s: %s\n", path, dlerror());
			return -1;
		}
	#endif

	// Call plugin init() function
	typedef Plugin *(*InitCallback)();
	InitCallback initCallback;
	#if defined(WINDOWS)
		initCallback = (InitCallback) GetProcAddress(handle, "init");
	#elif defined(LINUX) || defined(APPLE)
		initCallback = (InitCallback) dlsym(handle, "init");
	#endif
	if (!initCallback) {
		fprintf(stderr, "Failed to read init() symbol in %s\n", path);
		return -2;
	}

	// Add plugin to map
	Plugin *plugin = initCallback();
	if (!plugin) {
		fprintf(stderr, "Library %s did not return a plugin\n", path);
		return -3;
	}
	gPlugins.push_back(plugin);
	fprintf(stderr, "Loaded plugin %s\n", path);
	return 0;
}

void pluginInit() {
	curl_global_init(CURL_GLOBAL_ALL);

	// Load core
	// This function is defined in core.cpp
	Plugin *corePlugin = init();
	gPlugins.push_back(corePlugin);

	// Search for plugin libraries
	#if defined(WINDOWS)
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile("plugins/*", &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				char pluginFilename[MAX_PATH];
				snprintf(pluginFilename, sizeof(pluginFilename), "plugins/%s/plugin.dll", ffd.cFileName);
				loadPlugin(pluginFilename);
			} while (FindNextFile(hFind, &ffd));
		}
		FindClose(hFind);

	#elif defined(LINUX) || defined(APPLE)
		#if defined(LINUX)
			const char *globPath = "plugins/*/plugin.so";
		#elif defined(WINDOWS)
			const char *globPath = "plugins/*/plugin.dll";
		#elif defined(APPLE)
			const char *globPath = "plugins/*/plugin.dylib";
		#endif
		glob_t result;
		glob(globPath, GLOB_TILDE, NULL, &result);
		for (int i = 0; i < (int) result.gl_pathc; i++) {
			loadPlugin(result.gl_pathv[i]);
		}
		globfree(&result);
	#endif
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

static void extract_zip(int zipfd, int dirfd) {
	int err = 0;
	zip_t *za = zip_fdopen(zipfd, 0, &err);
	if (!za) return;
	if (err) goto cleanup;

	for (int i = 0; i < zip_get_num_entries(za, 0); i++) {
		zip_stat_t zs;
		err = zip_stat_index(za, i, 0, &zs);
		if (err) goto cleanup;
		int nameLen = strlen(zs.name);

		if (zs.name[nameLen - 1] == '/') {
			err = mkdirat(dirfd, zs.name, 0755);
			if (err) goto cleanup;
		}
		else {
			zip_file_t *zf = zip_fopen_index(za, i, 0);
			if (!zf) goto cleanup;

			int out = openat(dirfd, zs.name, O_RDWR | O_TRUNC | O_CREAT, 0644);
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
#if defined(LINUX)
	std::string command = "xdg-open " + url;
	system(command.c_str());
#endif
#if defined(APPLE)
	std::string command = "open " + url;
	system(command.c_str());
#endif
#if defined(WINDOWS)
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
		json_t *root = json_loads(resText.c_str(), 0, &error);
		if (root) {
			json_t *tokenJ = json_object_get(root, "token");
			if (tokenJ) {
				// Set the token, which logs the user in
				token = json_string_value(tokenJ);
			}
			json_decref(root);
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

	std::string filename = slug + ".zip";
	int dir = open("plugins", O_RDONLY | O_DIRECTORY);
	int zip = openat(dir, filename.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
	// Download zip
	download_file(zip, url.c_str());
	// Unzip file
	lseek(zip, 0, SEEK_SET);
	extract_zip(zip, dir);
	// Close files
	close(zip);
	close(dir);
	downloadName = "";
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
		json_t *root = json_loads(resText.c_str(), 0, &error);
		if (root) {
			json_t *pluginsJ = json_object_get(root, "plugins");
			if (pluginsJ) {
				// Iterate through each plugin object
				size_t index;
				json_t *pluginJ;
				json_array_foreach(pluginsJ, index, pluginJ) {
					pluginRefreshPlugin(pluginJ);
				}
			}
			json_decref(root);
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
