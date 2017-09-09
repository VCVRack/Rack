#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>

#include <zip.h>
#include <jansson.h>

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif
#include <dirent.h>

#include "plugin.hpp"
#include "util/request.hpp"


namespace rack {

std::list<Plugin*> gPlugins;

static std::string token;
static bool isDownloading = false;
static float downloadProgress = 0.0;
static std::string downloadName;
static std::string loginStatus;


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
	requestInit();

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
	requestDestroy();
}

////////////////////
// CURL and libzip helpers
////////////////////


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

static std::string apiHost = "http://api.vcvrack.com";

void pluginLogIn(std::string email, std::string password) {
	json_t *reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	json_t *resJ = requestJson(POST_METHOD, apiHost + "/token", reqJ);

	if (resJ) {
		// TODO parse response
		json_t *errorJ = json_object_get(resJ, "error");
		if (errorJ) {
			const char *errorStr = json_string_value(errorJ);
			loginStatus = errorStr;
		}
		else {
			json_t *tokenJ = json_object_get(resJ, "token");
			if (tokenJ) {
				const char *tokenStr = json_string_value(tokenJ);
				token = tokenStr;
				loginStatus = "";
			}
		}
		json_decref(resJ);
	}
	json_decref(reqJ);
}

void pluginLogOut() {
	token = "";
}

static void pluginRefreshPlugin(json_t *pluginJ) {
	// TODO
	// Refactor

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
	// download_file(zip, url.c_str());
	// Unzip file
	lseek(zip, 0, SEEK_SET);
	extract_zip(dir, zip);
	// Close file
	close(zip);
	downloadName = "";

	// Load plugin
}

void pluginRefresh() {
	// TODO
	// Refactor with requestJson()

	if (token.empty())
		return;

	/*
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
	*/
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

std::string pluginGetLoginStatus() {
	return loginStatus;
}


} // namespace rack
