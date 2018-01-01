#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#include <thread>

#include <zip.h>
#include <jansson.h>

#if defined(ARCH_WIN)
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif
#include <dirent.h>

#include "plugin.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "util/request.hpp"


namespace rack {

std::list<Plugin*> gPlugins;
std::string gToken;


static bool isDownloading = false;
static float downloadProgress = 0.0;
static std::string downloadName;
static std::string loginStatus;


Plugin::~Plugin() {
	for (Model *model : models) {
		delete model;
	}
}

void Plugin::addModel(Model *model) {
	assert(!model->plugin);
	model->plugin = this;
	models.push_back(model);
}


static int loadPlugin(std::string path) {
	std::string libraryFilename;
#if ARCH_LIN
	libraryFilename = path + "/" + "plugin.so";
#elif ARCH_WIN
	libraryFilename = path + "/" + "plugin.dll";
#elif ARCH_MAC
	libraryFilename = path + "/" + "plugin.dylib";
#endif

	// Load dynamic/shared library
#if ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE handle = LoadLibrary(libraryFilename.c_str());
	SetErrorMode(0);
	if (!handle) {
		int error = GetLastError();
		warn("Failed to load library %s: %d", libraryFilename.c_str(), error);
		return -1;
	}
#elif ARCH_LIN || ARCH_MAC
	void *handle = dlopen(libraryFilename.c_str(), RTLD_NOW);
	if (!handle) {
		warn("Failed to load library %s: %s", libraryFilename.c_str(), dlerror());
		return -1;
	}
#endif

	// Call plugin's init() function
	typedef void (*InitCallback)(Plugin *);
	InitCallback initCallback;
#if ARCH_WIN
	initCallback = (InitCallback) GetProcAddress(handle, "init");
#elif ARCH_LIN || ARCH_MAC
	initCallback = (InitCallback) dlsym(handle, "init");
#endif
	if (!initCallback) {
		warn("Failed to read init() symbol in %s", libraryFilename.c_str());
		return -2;
	}

	// Construct and initialize Plugin instance
	Plugin *plugin = new Plugin();
	plugin->path = path;
	plugin->handle = handle;
	initCallback(plugin);

	// Reject plugin if slug already exists
	for (Plugin *p : gPlugins) {
		if (plugin->slug == p->slug) {
			warn("Plugin \"%s\" is already loaded, not attempting to load it again");
			// TODO
			// Fix memory leak with `plugin` here
			return -1;
		}
	}

	// Add plugin to list
	gPlugins.push_back(plugin);
	info("Loaded plugin %s", libraryFilename.c_str());

	return 0;
}

static void loadPlugins(std::string path) {
	DIR *dir = opendir(path.c_str());
	if (dir) {
		struct dirent *d;
		while ((d = readdir(dir))) {
			if (d->d_name[0] == '.')
				continue;
			loadPlugin(path + "/" + d->d_name);
		}
		closedir(dir);
	}
}

////////////////////
// plugin helpers
////////////////////

static int extractZipHandle(zip_t *za, const char *dir) {
	int err = 0;
	for (int i = 0; i < zip_get_num_entries(za, 0); i++) {
		zip_stat_t zs;
		err = zip_stat_index(za, i, 0, &zs);
		if (err)
			return err;
		int nameLen = strlen(zs.name);

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", dir, zs.name);

		if (zs.name[nameLen - 1] == '/') {
			err = mkdir(path, 0755);
			if (err && errno != EEXIST)
				return err;
		}
		else {
			zip_file_t *zf = zip_fopen_index(za, i, 0);
			if (!zf)
				return 1;

			FILE *outFile = fopen(path, "wb");
			if (!outFile)
				continue;

			while (1) {
				char buffer[4096];
				int len = zip_fread(zf, buffer, sizeof(buffer));
				if (len <= 0)
					break;
				fwrite(buffer, 1, len, outFile);
			}

			err = zip_fclose(zf);
			if (err)
				return err;
			fclose(outFile);
		}
	}
	return 0;
}

static int extractZip(const char *filename, const char *dir) {
	int err = 0;
	zip_t *za = zip_open(filename, 0, &err);
	if (!za)
		return 1;

	if (!err) {
		err = extractZipHandle(za, dir);
	}

	zip_close(za);
	return err;
}

static void syncPlugin(json_t *pluginJ) {
	json_t *slugJ = json_object_get(pluginJ, "slug");
	if (!slugJ) return;
	std::string slug = json_string_value(slugJ);
	info("Syncing plugin %s", slug.c_str());

	json_t *nameJ = json_object_get(pluginJ, "name");
	if (!nameJ) return;
	std::string name = json_string_value(nameJ);

	std::string download;
	std::string sha256;

	json_t *downloadsJ = json_object_get(pluginJ, "downloads");
	if (downloadsJ) {
#if defined(ARCH_WIN)
	#define DOWNLOADS_ARCH "win"
#elif defined(ARCH_MAC)
	#define DOWNLOADS_ARCH "mac"
#elif defined(ARCH_LIN)
	#define DOWNLOADS_ARCH "lin"
#endif
		json_t *archJ = json_object_get(downloadsJ, DOWNLOADS_ARCH);
		if (archJ) {
			// Get download URL
			json_t *downloadJ = json_object_get(archJ, "download");
			if (downloadJ)
				download = json_string_value(downloadJ);
			// Get SHA256 hash
			json_t *sha256J = json_object_get(archJ, "sha256");
			if (sha256J)
				sha256 = json_string_value(sha256J);
		}
	}

	json_t *productIdJ = json_object_get(pluginJ, "productId");
	if (productIdJ) {
		download = gApiHost;
		download += "/download";
		download += "?slug=";
		download += slug;
		download += "&token=";
		download += requestEscape(gToken);
	}

	if (download.empty()) {
		warn("Could not get download URL for plugin %s", slug.c_str());
		return;
	}

	// If plugin is not loaded, download the zip file to /plugins
	downloadName = name;
	downloadProgress = 0.0;

	// Download zip
	std::string pluginsDir = assetLocal("plugins");
	std::string pluginPath = pluginsDir + "/" + slug;
	std::string zipPath = pluginPath + ".zip";
	bool success = requestDownload(download, zipPath, &downloadProgress);
	if (success) {
		if (!sha256.empty()) {
			// Check SHA256 hash
			std::string actualSha256 = requestSHA256File(zipPath);
			if (actualSha256 != sha256) {
				warn("Plugin %s does not match expected SHA256 checksum", slug.c_str());
				return;
			}
		}

		// Unzip file
		int err = extractZip(zipPath.c_str(), pluginsDir.c_str());
		if (!err) {
			// Delete zip
			remove(zipPath.c_str());
			// Load plugin
			// loadPlugin(pluginPath);
		}
	}

	downloadName = "";
}

static bool trySyncPlugin(json_t *pluginJ, json_t *communityPluginsJ, bool dryRun) {
	std::string slug = json_string_value(pluginJ);

	// Find community plugin
	size_t communityIndex;
	json_t *communityPluginJ = NULL;
	json_array_foreach(communityPluginsJ, communityIndex, communityPluginJ) {
		json_t *communitySlugJ = json_object_get(communityPluginJ, "slug");
		if (communitySlugJ) {
			std::string communitySlug = json_string_value(communitySlugJ);
			if (slug == communitySlug)
				break;
		}
	}
	if (communityIndex == json_array_size(communityPluginsJ)) {
		warn("Plugin sync error: %s not found in community", slug.c_str());
		return false;
	}

	// Get community version
	std::string version;
	json_t *versionJ = json_object_get(communityPluginJ, "version");
	if (versionJ) {
		version = json_string_value(versionJ);
	}

	// Check whether we already have a plugin with the same slug and version
	for (Plugin *plugin : gPlugins) {
		if (plugin->slug == slug) {
			// plugin->version might be blank, so adding a version of the manifest will update the plugin
			if (plugin->version == version)
				return false;
		}
	}

	if (!dryRun)
		syncPlugin(communityPluginJ);
	return true;
}

bool pluginSync(bool dryRun) {
	if (gToken.empty())
		return false;

	bool available = false;

	// Download my plugins
	json_t *reqJ = json_object();
	json_object_set(reqJ, "version", json_string(gApplicationVersion.c_str()));
	json_object_set(reqJ, "token", json_string(gToken.c_str()));
	json_t *resJ = requestJson(METHOD_GET, gApiHost + "/plugins", reqJ);
	json_decref(reqJ);

	// Download community plugins
	json_t *communityResJ = requestJson(METHOD_GET, gApiHost + "/community/plugins", NULL);

	if (!dryRun) {
		isDownloading = true;
		downloadProgress = 0.0;
		downloadName = "";
	}

	if (resJ && communityResJ) {
		json_t *errorJ = json_object_get(resJ, "error");
		json_t *communityErrorJ = json_object_get(resJ, "error");
		if (errorJ) {
			warn("Plugin sync error: %s", json_string_value(errorJ));
		}
		else if (communityErrorJ) {
			warn("Plugin sync error: %s", json_string_value(communityErrorJ));
		}
		else {
			// Check each plugin in list of my plugins
			json_t *pluginsJ = json_object_get(resJ, "plugins");
			json_t *communityPluginsJ = json_object_get(communityResJ, "plugins");
			size_t index;
			json_t *pluginJ;
			json_array_foreach(pluginsJ, index, pluginJ) {
				if (trySyncPlugin(pluginJ, communityPluginsJ, dryRun))
					available = true;
			}
		}
	}

	if (resJ)
		json_decref(resJ);

	if (communityResJ)
		json_decref(communityResJ);

	if (!dryRun) {
		isDownloading = false;
	}

	return available;
}

////////////////////
// plugin API
////////////////////

void pluginInit() {
	tagsInit();
	// Load core
	// This function is defined in core.cpp
	Plugin *coreManufacturer = new Plugin();
	init(coreManufacturer);
	gPlugins.push_back(coreManufacturer);

	// Load plugins from global directory
	std::string globalPlugins = assetGlobal("plugins");
	info("Loading plugins from %s", globalPlugins.c_str());
	loadPlugins(globalPlugins);

	// Load plugins from local directory
	std::string localPlugins = assetLocal("plugins");
	if (globalPlugins != localPlugins) {
		mkdir(localPlugins.c_str(), 0755);
		info("Loading plugins from %s", localPlugins.c_str());
		loadPlugins(localPlugins);
	}
}

void pluginDestroy() {
	for (Plugin *plugin : gPlugins) {
		// Free library handle
#if defined(ARCH_WIN)
		if (plugin->handle)
			FreeLibrary((HINSTANCE)plugin->handle);
#elif defined(ARCH_LIN) || defined(ARCH_MAC)
		if (plugin->handle)
			dlclose(plugin->handle);
#endif

		// For some reason this segfaults.
		// It might be best to let them leak anyway, because "crash on exit" issues would occur with badly-written plugins.
		// delete plugin;
	}
	gPlugins.clear();
}

void pluginLogIn(std::string email, std::string password) {
	json_t *reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	json_t *resJ = requestJson(METHOD_POST, gApiHost + "/token", reqJ);
	json_decref(reqJ);

	if (resJ) {
		json_t *errorJ = json_object_get(resJ, "error");
		if (errorJ) {
			const char *errorStr = json_string_value(errorJ);
			loginStatus = errorStr;
		}
		else {
			json_t *tokenJ = json_object_get(resJ, "token");
			if (tokenJ) {
				const char *tokenStr = json_string_value(tokenJ);
				gToken = tokenStr;
				loginStatus = "";
			}
		}
		json_decref(resJ);
	}
}

void pluginLogOut() {
	gToken = "";
}

void pluginCancelDownload() {
	// TODO
}

bool pluginIsLoggedIn() {
	return gToken != "";
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
