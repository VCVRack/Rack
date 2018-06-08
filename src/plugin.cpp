#include "plugin.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "util/request.hpp"
#include "osdialog.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#include <thread>
#include <stdexcept>

#define ZIP_STATIC
#include <zip.h>
#include <jansson.h>

#if ARCH_WIN
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif
#include <dirent.h>


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

////////////////////
// private API
////////////////////

static bool loadPlugin(std::string path) {
	std::string libraryFilename;
#if ARCH_LIN
	libraryFilename = path + "/" + "plugin.so";
#elif ARCH_WIN
	libraryFilename = path + "/" + "plugin.dll";
#elif ARCH_MAC
	libraryFilename = path + "/" + "plugin.dylib";
#endif

	// Check file existence
	if (!systemIsFile(libraryFilename)) {
		warn("Plugin file %s does not exist", libraryFilename.c_str());
		return false;
	}

	// Load dynamic/shared library
#if ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE handle = LoadLibrary(libraryFilename.c_str());
	SetErrorMode(0);
	if (!handle) {
		int error = GetLastError();
		warn("Failed to load library %s: code %d", libraryFilename.c_str(), error);
		return false;
	}
#else
	void *handle = dlopen(libraryFilename.c_str(), RTLD_NOW);
	if (!handle) {
		warn("Failed to load library %s: %s", libraryFilename.c_str(), dlerror());
		return false;
	}
#endif

	// Call plugin's init() function
	typedef void (*InitCallback)(Plugin *);
	InitCallback initCallback;
#if ARCH_WIN
	initCallback = (InitCallback) GetProcAddress(handle, "init");
#else
	initCallback = (InitCallback) dlsym(handle, "init");
#endif
	if (!initCallback) {
		warn("Failed to read init() symbol in %s", libraryFilename.c_str());
		return false;
	}

	// Construct and initialize Plugin instance
	Plugin *plugin = new Plugin();
	plugin->path = path;
	plugin->handle = handle;
	initCallback(plugin);

	// Reject plugin if slug already exists
	Plugin *oldPlugin = pluginGetPlugin(plugin->slug);
	if (oldPlugin) {
		warn("Plugin \"%s\" is already loaded, not attempting to load it again", plugin->slug.c_str());
		// TODO
		// Fix memory leak with `plugin` here
		return false;
	}

	// Add plugin to list
	gPlugins.push_back(plugin);
	info("Loaded plugin %s %s from %s", plugin->slug.c_str(), plugin->version.c_str(), libraryFilename.c_str());

	return true;
}

static bool syncPlugin(std::string slug, json_t *manifestJ, bool dryRun) {
	// Check that "status" is "available"
	json_t *statusJ = json_object_get(manifestJ, "status");
	if (!statusJ) {
		return false;
	}
	std::string status = json_string_value(statusJ);
	if (status != "available") {
		return false;
	}

	// Get latest version
	json_t *latestVersionJ = json_object_get(manifestJ, "latestVersion");
	if (!latestVersionJ) {
		warn("Could not get latest version of plugin %s", slug.c_str());
		return false;
	}
	std::string latestVersion = json_string_value(latestVersionJ);

	// Check whether we already have a plugin with the same slug and version
	Plugin *plugin = pluginGetPlugin(slug);
	if (plugin && plugin->version == latestVersion) {
		return false;
	}

	json_t *nameJ = json_object_get(manifestJ, "name");
	std::string name;
	if (nameJ) {
		name = json_string_value(nameJ);
	}
	else {
		name = slug;
	}

#if ARCH_WIN
	std::string arch = "win";
#elif ARCH_MAC
	std::string arch = "mac";
#elif ARCH_LIN
	std::string arch = "lin";
#endif

	std::string downloadUrl;
	downloadUrl = gApiHost;
	downloadUrl += "/download";
	if (dryRun) {
		downloadUrl += "/available";
	}
	downloadUrl += "?token=" + requestEscape(gToken);
	downloadUrl += "&slug=" + requestEscape(slug);
	downloadUrl += "&version=" + requestEscape(latestVersion);
	downloadUrl += "&arch=" + requestEscape(arch);

	if (dryRun) {
		// Check if available
		json_t *availableResJ = requestJson(METHOD_GET, downloadUrl, NULL);
		if (!availableResJ) {
			warn("Could not check whether download is available");
			return false;
		}
		defer({
			json_decref(availableResJ);
		});
		json_t *successJ = json_object_get(availableResJ, "success");
		return json_boolean_value(successJ);
	}
	else {
		downloadName = name;
		downloadProgress = 0.0;
		info("Downloading plugin %s %s %s", slug.c_str(), latestVersion.c_str(), arch.c_str());

		// Download zip
		std::string pluginDest = assetLocal("plugins/" + slug + ".zip");
		if (!requestDownload(downloadUrl, pluginDest, &downloadProgress)) {
			warn("Plugin %s download was unsuccessful", slug.c_str());
			return false;
		}

		downloadName = "";
		return true;
	}
}

static void loadPlugins(std::string path) {
	std::string message;
	for (std::string pluginPath : systemListEntries(path)) {
		if (!systemIsDirectory(pluginPath))
			continue;
		if (!loadPlugin(pluginPath)) {
			message += stringf("Could not load plugin %s\n", pluginPath.c_str());
		}
	}
	if (!message.empty()) {
		message += "See log for details.";
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
}

/** Returns 0 if successful */
static int extractZipHandle(zip_t *za, const char *dir) {
	int err;
	for (int i = 0; i < zip_get_num_entries(za, 0); i++) {
		zip_stat_t zs;
		err = zip_stat_index(za, i, 0, &zs);
		if (err) {
			warn("zip_stat_index() failed: error %d", err);
			return err;
		}
		int nameLen = strlen(zs.name);

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", dir, zs.name);

		if (zs.name[nameLen - 1] == '/') {
			if (mkdir(path, 0755)) {
				if (errno != EEXIST) {
					warn("mkdir(%s) failed: error %d", path, errno);
					return errno;
				}
			}
		}
		else {
			zip_file_t *zf = zip_fopen_index(za, i, 0);
			if (!zf) {
				warn("zip_fopen_index() failed");
				return -1;
			}

			FILE *outFile = fopen(path, "wb");
			if (!outFile)
				continue;

			while (1) {
				char buffer[1<<15];
				int len = zip_fread(zf, buffer, sizeof(buffer));
				if (len <= 0)
					break;
				fwrite(buffer, 1, len, outFile);
			}

			err = zip_fclose(zf);
			if (err) {
				warn("zip_fclose() failed: error %d", err);
				return err;
			}
			fclose(outFile);
		}
	}
	return 0;
}

/** Returns 0 if successful */
static int extractZip(const char *filename, const char *path) {
	int err;
	zip_t *za = zip_open(filename, 0, &err);
	if (!za) {
		warn("Could not open zip %s: error %d", filename, err);
		return err;
	}
	defer({
		zip_close(za);
	});

	err = extractZipHandle(za, path);
	return err;
}

static void extractPackages(std::string path) {
	std::string message;

	for (std::string packagePath : systemListEntries(path)) {
		if (stringExtension(packagePath) != "zip")
			continue;
		info("Extracting package %s", packagePath.c_str());
		// Extract package
		if (extractZip(packagePath.c_str(), path.c_str())) {
			warn("Package %s failed to extract", packagePath.c_str());
			message += stringf("Could not extract package %s\n", packagePath.c_str());
			continue;
		}
		// Remove package
		if (remove(packagePath.c_str())) {
			warn("Could not delete file %s: error %d", packagePath.c_str(), errno);
		}
	}
	if (!message.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
}

////////////////////
// public API
////////////////////

void pluginInit(bool devMode) {
	tagsInit();

	// Load core
	// This function is defined in core.cpp
	Plugin *corePlugin = new Plugin();
	init(corePlugin);
	gPlugins.push_back(corePlugin);

	// Get local plugins directory
	std::string localPlugins = assetLocal("plugins");
	mkdir(localPlugins.c_str(), 0755);

	if (!devMode) {
		// Copy Fundamental package to plugins directory if folder does not exist
		std::string fundamentalSrc = assetGlobal("Fundamental.zip");
		std::string fundamentalDest = assetLocal("plugins/Fundamental.zip");
		std::string fundamentalDir = assetLocal("plugins/Fundamental");
		if (systemIsFile(fundamentalSrc) && !systemIsFile(fundamentalDest) && !systemIsDirectory(fundamentalDir)) {
			systemCopy(fundamentalSrc, fundamentalDest);
		}
	}

	// Extract packages and load plugins
	extractPackages(localPlugins);
	loadPlugins(localPlugins);
}

void pluginDestroy() {
	for (Plugin *plugin : gPlugins) {
		// Free library handle
#if ARCH_WIN
		if (plugin->handle)
			FreeLibrary((HINSTANCE)plugin->handle);
#else
		if (plugin->handle)
			dlclose(plugin->handle);
#endif

		// For some reason this segfaults.
		// It might be best to let them leak anyway, because "crash on exit" issues would occur with badly-written plugins.
		// delete plugin;
	}
	gPlugins.clear();
}

bool pluginSync(bool dryRun) {
	if (gToken.empty())
		return false;

	bool available = false;

	if (!dryRun) {
		isDownloading = true;
		downloadProgress = 0.0;
		downloadName = "Updating plugins...";
	}
	defer({
		isDownloading = false;
	});

	// Get user's plugins list
	json_t *pluginsReqJ = json_object();
	json_object_set(pluginsReqJ, "token", json_string(gToken.c_str()));
	json_t *pluginsResJ = requestJson(METHOD_GET, gApiHost + "/plugins", pluginsReqJ);
	json_decref(pluginsReqJ);
	if (!pluginsResJ) {
		warn("Request for user's plugins failed");
		return false;
	}
	defer({
		json_decref(pluginsResJ);
	});

	json_t *errorJ = json_object_get(pluginsResJ, "error");
	if (errorJ) {
		warn("Request for user's plugins returned an error: %s", json_string_value(errorJ));
		return false;
	}

	// Get community manifests
	json_t *manifestsResJ = requestJson(METHOD_GET, gApiHost + "/community/manifests", NULL);
	if (!manifestsResJ) {
		warn("Request for community manifests failed");
		return false;
	}
	defer({
		json_decref(manifestsResJ);
	});

	// Check each plugin in list of plugin slugs
	json_t *pluginsJ = json_object_get(pluginsResJ, "plugins");
	if (!pluginsJ) {
		warn("No plugins array");
		return false;
	}
	json_t *manifestsJ = json_object_get(manifestsResJ, "manifests");
	if (!manifestsJ) {
		warn("No manifests object");
		return false;
	}

	size_t slugIndex;
	json_t *slugJ;
	json_array_foreach(pluginsJ, slugIndex, slugJ) {
		std::string slug = json_string_value(slugJ);
		// Search for slug in manifests
		const char *manifestSlug;
		json_t *manifestJ = NULL;
		json_object_foreach(manifestsJ, manifestSlug, manifestJ) {
			if (slug == std::string(manifestSlug))
				break;
		}

		if (!manifestJ)
			continue;

		if (syncPlugin(slug, manifestJ, dryRun)) {
			available = true;
		}
	}

	return available;
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

Plugin *pluginGetPlugin(std::string pluginSlug) {
	for (Plugin *plugin : gPlugins) {
		if (plugin->slug == pluginSlug) {
			return plugin;
		}
	}
	return NULL;
}

Model *pluginGetModel(std::string pluginSlug, std::string modelSlug) {
	Plugin *plugin = pluginGetPlugin(pluginSlug);
	if (plugin) {
		for (Model *model : plugin->models) {
			if (model->slug == modelSlug) {
				return model;
			}
		}
	}
	return NULL;
}


} // namespace rack
