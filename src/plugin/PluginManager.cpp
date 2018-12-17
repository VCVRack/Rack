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

#include "plugin/PluginManager.hpp"
#include "system.hpp"
#include "logger.hpp"
#include "network.hpp"
#include "asset.hpp"
#include "string.hpp"
#include "app/common.hpp"
#include "osdialog.h"


namespace rack {


////////////////////
// private API
////////////////////

static bool PluginManager_loadPlugin(PluginManager *pluginManager, std::string path) {
	std::string libraryFilename;
#if ARCH_LIN
	libraryFilename = path + "/" + "plugin.so";
#elif ARCH_WIN
	libraryFilename = path + "/" + "plugin.dll";
#elif ARCH_MAC
	libraryFilename = path + "/" + "plugin.dylib";
#endif

	// Check file existence
	if (!system::isFile(libraryFilename)) {
		WARN("Plugin file %s does not exist", libraryFilename.c_str());
		return false;
	}

	// Load dynamic/shared library
#if ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE handle = LoadLibrary(libraryFilename.c_str());
	SetErrorMode(0);
	if (!handle) {
		int error = GetLastError();
		WARN("Failed to load library %s: code %d", libraryFilename.c_str(), error);
		return false;
	}
#else
	void *handle = dlopen(libraryFilename.c_str(), RTLD_NOW);
	if (!handle) {
		WARN("Failed to load library %s: %s", libraryFilename.c_str(), dlerror());
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
		WARN("Failed to read init() symbol in %s", libraryFilename.c_str());
		return false;
	}

	// Construct and initialize Plugin instance
	Plugin *plugin = new Plugin;
	plugin->path = path;
	plugin->handle = handle;
	initCallback(plugin);

	// Reject plugin if slug already exists
	Plugin *oldPlugin = pluginManager->getPlugin(plugin->slug);
	if (oldPlugin) {
		WARN("Plugin \"%s\" is already loaded, not attempting to load it again", plugin->slug.c_str());
		// TODO
		// Fix memory leak with `plugin` here
		return false;
	}

	// Add plugin to list
	pluginManager->plugins.push_back(plugin);
	INFO("Loaded plugin %s %s from %s", plugin->slug.c_str(), plugin->version.c_str(), libraryFilename.c_str());

	return true;
}

static bool PluginManager_syncPlugin(PluginManager *pluginManager, std::string slug, json_t *manifestJ, bool dryRun) {
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
		WARN("Could not get latest version of plugin %s", slug.c_str());
		return false;
	}
	std::string latestVersion = json_string_value(latestVersionJ);

	// Check whether we already have a plugin with the same slug and version
	Plugin *plugin = pluginManager->getPlugin(slug);
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
	downloadUrl = API_HOST;
	downloadUrl += "/download";
	if (dryRun) {
		downloadUrl += "/available";
	}
	downloadUrl += "?token=" + network::encodeUrl(pluginManager->token);
	downloadUrl += "&slug=" + network::encodeUrl(slug);
	downloadUrl += "&version=" + network::encodeUrl(latestVersion);
	downloadUrl += "&arch=" + network::encodeUrl(arch);

	if (dryRun) {
		// Check if available
		json_t *availableResJ = network::requestJson(network::METHOD_GET, downloadUrl, NULL);
		if (!availableResJ) {
			WARN("Could not check whether download is available");
			return false;
		}
		DEFER({
			json_decref(availableResJ);
		});
		json_t *successJ = json_object_get(availableResJ, "success");
		return json_boolean_value(successJ);
	}
	else {
		pluginManager->downloadName = name;
		pluginManager->downloadProgress = 0.0;
		INFO("Downloading plugin %s %s %s", slug.c_str(), latestVersion.c_str(), arch.c_str());

		// Download zip
		std::string pluginDest = asset::local("plugins/" + slug + ".zip");
		if (!network::requestDownload(downloadUrl, pluginDest, &pluginManager->downloadProgress)) {
			WARN("Plugin %s download was unsuccessful", slug.c_str());
			return false;
		}

		pluginManager->downloadName = "";
		return true;
	}
}

static void PluginManager_loadPlugins(PluginManager *pluginManager, std::string path) {
	std::string message;
	for (std::string pluginPath : system::listEntries(path)) {
		if (!system::isDirectory(pluginPath))
			continue;
		if (!PluginManager_loadPlugin(pluginManager, pluginPath)) {
			message += string::f("Could not load plugin %s\n", pluginPath.c_str());
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
			WARN("zip_stat_index() failed: error %d", err);
			return err;
		}
		int nameLen = strlen(zs.name);

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", dir, zs.name);

		if (zs.name[nameLen - 1] == '/') {
			if (mkdir(path, 0755)) {
				if (errno != EEXIST) {
					WARN("mkdir(%s) failed: error %d", path, errno);
					return errno;
				}
			}
		}
		else {
			zip_file_t *zf = zip_fopen_index(za, i, 0);
			if (!zf) {
				WARN("zip_fopen_index() failed");
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
				WARN("zip_fclose() failed: error %d", err);
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
		WARN("Could not open zip %s: error %d", filename, err);
		return err;
	}
	DEFER({
		zip_close(za);
	});

	err = extractZipHandle(za, path);
	return err;
}

static void extractPackages(std::string path) {
	std::string message;

	for (std::string packagePath : system::listEntries(path)) {
		if (string::extension(packagePath) != "zip")
			continue;
		INFO("Extracting package %s", packagePath.c_str());
		// Extract package
		if (extractZip(packagePath.c_str(), path.c_str())) {
			WARN("Package %s failed to extract", packagePath.c_str());
			message += string::f("Could not extract package %s\n", packagePath.c_str());
			continue;
		}
		// Remove package
		if (remove(packagePath.c_str())) {
			WARN("Could not delete file %s: error %d", packagePath.c_str(), errno);
		}
	}
	if (!message.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
}

////////////////////
// public API
////////////////////

PluginManager::PluginManager(bool devMode) {
	// Load core
	// This function is defined in core.cpp
	Plugin *corePlugin = new Plugin;
	init(corePlugin);
	plugins.push_back(corePlugin);

	// Get local plugins directory
	std::string localPlugins = asset::local("plugins");
	mkdir(localPlugins.c_str(), 0755);

	if (!devMode) {
		// Copy Fundamental package to plugins directory if folder does not exist
		std::string fundamentalSrc = asset::global("Fundamental.zip");
		std::string fundamentalDest = asset::local("plugins/Fundamental.zip");
		std::string fundamentalDir = asset::local("plugins/Fundamental");
		if (system::isFile(fundamentalSrc) && !system::isFile(fundamentalDest) && !system::isDirectory(fundamentalDir)) {
			system::copyFile(fundamentalSrc, fundamentalDest);
		}
	}

	// Extract packages and load plugins
	extractPackages(localPlugins);
	PluginManager_loadPlugins(this, localPlugins);
}

PluginManager::~PluginManager() {
	for (Plugin *plugin : plugins) {
		// Free library handle
#if ARCH_WIN
		if (plugin->handle)
			FreeLibrary((HINSTANCE) plugin->handle);
#else
		if (plugin->handle)
			dlclose(plugin->handle);
#endif

		// For some reason this segfaults.
		// It might be best to let them leak anyway, because "crash on exit" issues would occur with badly-written plugins.
		// delete plugin;
	}
	plugins.clear();
}

void PluginManager::logIn(std::string email, std::string password) {
	json_t *reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	json_t *resJ = network::requestJson(network::METHOD_POST, API_HOST + "/token", reqJ);
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
				token = tokenStr;
				loginStatus = "";
			}
		}
		json_decref(resJ);
	}
}

void PluginManager::logOut() {
	token = "";
}

bool PluginManager::sync(bool dryRun) {
	if (token.empty())
		return false;

	bool available = false;

	if (!dryRun) {
		isDownloading = true;
		downloadProgress = 0.0;
		downloadName = "Updating plugins...";
	}
	DEFER({
		isDownloading = false;
	});

	// Get user's plugins list
	json_t *pluginsReqJ = json_object();
	json_object_set(pluginsReqJ, "token", json_string(token.c_str()));
	json_t *pluginsResJ = network::requestJson(network::METHOD_GET, API_HOST + "/plugins", pluginsReqJ);
	json_decref(pluginsReqJ);
	if (!pluginsResJ) {
		WARN("Request for user's plugins failed");
		return false;
	}
	DEFER({
		json_decref(pluginsResJ);
	});

	json_t *errorJ = json_object_get(pluginsResJ, "error");
	if (errorJ) {
		WARN("Request for user's plugins returned an error: %s", json_string_value(errorJ));
		return false;
	}

	// Get community manifests
	json_t *manifestsResJ = network::requestJson(network::METHOD_GET, API_HOST + "/community/manifests", NULL);
	if (!manifestsResJ) {
		WARN("Request for community manifests failed");
		return false;
	}
	DEFER({
		json_decref(manifestsResJ);
	});

	// Check each plugin in list of plugin slugs
	json_t *pluginsJ = json_object_get(pluginsResJ, "plugins");
	if (!pluginsJ) {
		WARN("No plugins array");
		return false;
	}
	json_t *manifestsJ = json_object_get(manifestsResJ, "manifests");
	if (!manifestsJ) {
		WARN("No manifests object");
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

		if (PluginManager_syncPlugin(this, slug, manifestJ, dryRun)) {
			available = true;
		}
	}

	return available;
}

void PluginManager::cancelDownload() {
	// TODO
}

bool PluginManager::isLoggedIn() {
	return token != "";
}

Plugin *PluginManager::getPlugin(std::string pluginSlug) {
	for (Plugin *plugin : plugins) {
		if (plugin->slug == pluginSlug) {
			return plugin;
		}
	}
	return NULL;
}

Model *PluginManager::getModel(std::string pluginSlug, std::string modelSlug) {
	Plugin *plugin = getPlugin(pluginSlug);
	if (plugin) {
		for (Model *model : plugin->models) {
			if (model->slug == modelSlug) {
				return model;
			}
		}
	}
	return NULL;
}


PluginManager *gPluginManager = NULL;


} // namespace rack
