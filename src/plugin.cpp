#include <plugin.hpp>
#include <system.hpp>
#include <network.hpp>
#include <asset.hpp>
#include <string.hpp>
#include <app.hpp>
#include <app/common.hpp>
#include <plugin/callbacks.hpp>
#include <settings.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#include <thread>
#include <map>
#include <stdexcept>

#include <jansson.h>

#if defined ARCH_WIN
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif
#include <dirent.h>
#include <osdialog.h>


namespace rack {

namespace core {
void init(rack::plugin::Plugin* plugin);
} // namespace core

namespace plugin {


////////////////////
// private API
////////////////////

typedef void (*InitCallback)(Plugin*);

static InitCallback loadLibrary(Plugin* plugin) {
	// Load plugin library
	std::string libraryFilename;
#if defined ARCH_LIN
	libraryFilename = plugin->path + "/" + "plugin.so";
#elif defined ARCH_WIN
	libraryFilename = plugin->path + "/" + "plugin.dll";
#elif ARCH_MAC
	libraryFilename = plugin->path + "/" + "plugin.dylib";
#endif

	// Check file existence
	if (!system::isFile(libraryFilename)) {
		throw UserException(string::f("Library %s does not exist", libraryFilename.c_str()));
	}

	// Load dynamic/shared library
#if defined ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE handle = LoadLibrary(libraryFilename.c_str());
	SetErrorMode(0);
	if (!handle) {
		int error = GetLastError();
		throw UserException(string::f("Failed to load library %s: code %d", libraryFilename.c_str(), error));
	}
#else
	void* handle = dlopen(libraryFilename.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		throw UserException(string::f("Failed to load library %s: %s", libraryFilename.c_str(), dlerror()));
	}
#endif
	plugin->handle = handle;

	// Get plugin's init() function
	InitCallback initCallback;
#if defined ARCH_WIN
	initCallback = (InitCallback) GetProcAddress(handle, "init");
#else
	initCallback = (InitCallback) dlsym(handle, "init");
#endif
	if (!initCallback) {
		throw UserException(string::f("Failed to read init() symbol in %s", libraryFilename.c_str()));
	}

	return initCallback;
}


/** If path is blank, loads Core */
static Plugin* loadPlugin(std::string path) {
	Plugin* plugin = new Plugin;
	try {
		plugin->path = path;

		// Get modified timestamp
		if (path != "") {
			struct stat statbuf;
			if (!stat(path.c_str(), &statbuf)) {
#if defined ARCH_MAC
				plugin->modifiedTimestamp = (double) statbuf.st_mtimespec.tv_sec + statbuf.st_mtimespec.tv_nsec * 1e-9;
#elif defined ARCH_WIN
				plugin->modifiedTimestamp = (double) statbuf.st_mtime;
#elif defined ARCH_LIN
				plugin->modifiedTimestamp = (double) statbuf.st_mtim.tv_sec + statbuf.st_mtim.tv_nsec * 1e-9;
#endif
			}
		}

		// Load plugin.json
		std::string manifestFilename = (path == "") ? asset::system("Core.json") : (path + "/plugin.json");
		FILE* file = fopen(manifestFilename.c_str(), "r");
		if (!file) {
			throw UserException(string::f("Manifest file %s does not exist", manifestFilename.c_str()));
		}
		DEFER({
			fclose(file);
		});

		json_error_t error;
		json_t* rootJ = json_loadf(file, 0, &error);
		if (!rootJ) {
			throw UserException(string::f("JSON parsing error at %s %d:%d %s", manifestFilename.c_str(), error.line, error.column, error.text));
		}
		DEFER({
			json_decref(rootJ);
		});

		// Call init callback
		InitCallback initCallback;
		if (path == "") {
			initCallback = core::init;
		}
		else {
			initCallback = loadLibrary(plugin);
		}
		initCallback(plugin);

		// Load manifest
		plugin->fromJson(rootJ);

		// Reject plugin if slug already exists
		Plugin* oldPlugin = getPlugin(plugin->slug);
		if (oldPlugin) {
			throw UserException(string::f("Plugin %s is already loaded, not attempting to load it again", plugin->slug.c_str()));
		}

		INFO("Loaded plugin %s v%s from %s", plugin->slug.c_str(), plugin->version.c_str(), path.c_str());
		plugins.push_back(plugin);
	}
	catch (UserException& e) {
		WARN("Could not load plugin %s: %s", path.c_str(), e.what());
		delete plugin;
		plugin = NULL;
	}
	return plugin;
}

static void loadPlugins(std::string path) {
	for (std::string pluginPath : system::getEntries(path)) {
		if (!system::isDirectory(pluginPath))
			continue;
		if (!loadPlugin(pluginPath)) {
			// Ignore bad plugins. They are reported in the log.
		}
	}
}

static void extractPackages(std::string path) {
	std::string message;

	for (std::string packagePath : system::getEntries(path)) {
		if (string::filenameExtension(string::filename(packagePath)) != "zip")
			continue;
		INFO("Extracting package %s", packagePath.c_str());
		// Extract package
		if (system::unzipToFolder(packagePath, path)) {
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

void init() {
	// Load Core
	loadPlugin("");

	// Get user plugins directory
	mkdir(asset::pluginsPath.c_str(), 0755);

	// Extract packages and load plugins
	extractPackages(asset::pluginsPath);
	loadPlugins(asset::pluginsPath);

	// If Fundamental wasn't loaded, copy the bundled Fundamental package and load it
#if defined ARCH_MAC
	std::string fundamentalSrc = asset::system("Fundamental.txt");
#else
	std::string fundamentalSrc = asset::system("Fundamental.zip");
#endif
	std::string fundamentalDir = asset::pluginsPath + "/Fundamental";
	if (!settings::devMode && !getPlugin("Fundamental") && system::isFile(fundamentalSrc)) {
		INFO("Extracting bundled Fundamental package");
		system::unzipToFolder(fundamentalSrc.c_str(), asset::pluginsPath.c_str());
		loadPlugin(fundamentalDir);
	}

	// Sync in a detached thread
	if (!settings::devMode) {
		std::thread t([] {
			queryUpdates();
		});
		t.detach();
	}
}

void destroy() {
	for (Plugin* plugin : plugins) {
		// Free library handle
#if defined ARCH_WIN
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

void logIn(const std::string& email, const std::string& password) {
	loginStatus = "Logging in...";
	json_t* reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	std::string url = app::API_URL + "/token";
	json_t* resJ = network::requestJson(network::METHOD_POST, url, reqJ);
	json_decref(reqJ);

	if (!resJ) {
		loginStatus = "No response from server";
		return;
	}
	DEFER({
		json_decref(resJ);
	});

	json_t* errorJ = json_object_get(resJ, "error");
	if (errorJ) {
		const char* errorStr = json_string_value(errorJ);
		loginStatus = errorStr;
		return;
	}

	json_t* tokenJ = json_object_get(resJ, "token");
	if (!tokenJ) {
		loginStatus = "No token in response";
		return;
	}

	const char* tokenStr = json_string_value(tokenJ);
	settings::token = tokenStr;
	loginStatus = "";
	queryUpdates();
}

void logOut() {
	settings::token = "";
	updates.clear();
}

bool isLoggedIn() {
	return settings::token != "";
}

void queryUpdates() {
	if (settings::token.empty())
		return;

	updates.clear();
	updateStatus = "Querying for updates...";

	// Get user's plugins list
	std::string pluginsUrl = app::API_URL + "/plugins";
	json_t* pluginsReqJ = json_object();
	json_object_set(pluginsReqJ, "token", json_string(settings::token.c_str()));
	json_t* pluginsResJ = network::requestJson(network::METHOD_GET, pluginsUrl, pluginsReqJ);
	json_decref(pluginsReqJ);
	if (!pluginsResJ) {
		WARN("Request for user's plugins failed");
		updateStatus = "Could not query updates";
		return;
	}
	DEFER({
		json_decref(pluginsResJ);
	});

	json_t* errorJ = json_object_get(pluginsResJ, "error");
	if (errorJ) {
		WARN("Request for user's plugins returned an error: %s", json_string_value(errorJ));
		updateStatus = "Could not query updates";
		return;
	}

	// Get library manifests
	std::string manifestsUrl = app::API_URL + "/library/manifests";
	json_t* manifestsReq = json_object();
	json_object_set(manifestsReq, "version", json_string(app::API_VERSION.c_str()));
	json_t* manifestsResJ = network::requestJson(network::METHOD_GET, manifestsUrl, manifestsReq);
	json_decref(manifestsReq);
	if (!manifestsResJ) {
		WARN("Request for library manifests failed");
		updateStatus = "Could not query updates";
		return;
	}
	DEFER({
		json_decref(manifestsResJ);
	});

	json_t* manifestsJ = json_object_get(manifestsResJ, "manifests");
	json_t* pluginsJ = json_object_get(pluginsResJ, "plugins");

	size_t pluginIndex;
	json_t* pluginJ;
	json_array_foreach(pluginsJ, pluginIndex, pluginJ) {
		Update update;
		// Get plugin manifest
		update.pluginSlug = json_string_value(pluginJ);
		json_t* manifestJ = json_object_get(manifestsJ, update.pluginSlug.c_str());
		if (!manifestJ) {
			WARN("VCV account has plugin %s but no manifest was found", update.pluginSlug.c_str());
			continue;
		}

		// Get plugin name
		json_t* nameJ = json_object_get(manifestJ, "name");
		if (nameJ)
			update.pluginName = json_string_value(nameJ);

		// Get version
		json_t* versionJ = json_object_get(manifestJ, "version");
		if (!versionJ) {
			WARN("Plugin %s has no version in manifest", update.pluginSlug.c_str());
			continue;
		}
		update.version = json_string_value(versionJ);

		// Check if update is needed
		Plugin* p = getPlugin(update.pluginSlug);
		if (p && p->version == update.version)
			continue;

		// Check status
		json_t* statusJ = json_object_get(manifestJ, "status");
		if (!statusJ)
			continue;
		std::string status = json_string_value(statusJ);
		if (status != "available")
			continue;

		// Get changelog URL
		json_t* changelogUrlJ = json_object_get(manifestJ, "changelogUrl");
		if (changelogUrlJ) {
			update.changelogUrl = json_string_value(changelogUrlJ);
		}

		updates.push_back(update);
	}

	updateStatus = "";
}

bool hasUpdates() {
	for (Update& update : updates) {
		if (update.progress < 1.f)
			return true;
	}
	return false;
}

static bool isSyncingUpdate = false;
static bool isSyncingUpdates = false;

void syncUpdate(Update* update) {
	isSyncingUpdate = true;
	DEFER({
		isSyncingUpdate = false;
	});

	std::string downloadUrl = app::API_URL + "/download";
	downloadUrl += "?token=" + network::encodeUrl(settings::token);
	downloadUrl += "&slug=" + network::encodeUrl(update->pluginSlug);
	downloadUrl += "&version=" + network::encodeUrl(update->version);
	downloadUrl += "&arch=" + network::encodeUrl(app::APP_ARCH);

	INFO("Downloading plugin %s %s %s", update->pluginSlug.c_str(), update->version.c_str(), app::APP_ARCH.c_str());

	// Download zip
	std::string pluginDest = asset::pluginsPath + "/" + update->pluginSlug + ".zip";
	if (!network::requestDownload(downloadUrl, pluginDest, &update->progress)) {
		WARN("Plugin %s download was unsuccessful", update->pluginSlug.c_str());
		return;
	}
}

void syncUpdates() {
	isSyncingUpdates = true;
	DEFER({
		isSyncingUpdates = false;
	});

	if (settings::token.empty())
		return;

	for (Update& update : updates) {
		if (update.progress < 1.f)
			syncUpdate(&update);
	}
	restartRequested = true;
}

bool isSyncing() {
	return isSyncingUpdate || isSyncingUpdates;
}

Plugin* getPlugin(const std::string& pluginSlug) {
	for (Plugin* plugin : plugins) {
		if (plugin->slug == pluginSlug) {
			return plugin;
		}
	}
	return NULL;
}

Model* getModel(const std::string& pluginSlug, const std::string& modelSlug) {
	Plugin* plugin = getPlugin(pluginSlug);
	if (!plugin)
		return NULL;
	Model* model = plugin->getModel(modelSlug);
	if (!model)
		return NULL;
	return model;
}

bool isSlugValid(const std::string& slug) {
	for (char c : slug) {
		if (!(std::isalnum(c) || c == '-' || c == '_'))
			return false;
	}
	return true;
}

std::string normalizeSlug(const std::string& slug) {
	std::string s;
	for (char c : slug) {
		if (!(std::isalnum(c) || c == '-' || c == '_'))
			continue;
		s += c;
	}
	return s;
}


std::vector<Plugin*> plugins;

std::string loginStatus;
std::vector<Update> updates;
std::string updateStatus;
bool restartRequested = false;


} // namespace plugin
} // namespace rack
