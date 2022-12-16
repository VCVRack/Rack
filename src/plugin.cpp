#include <thread>
#include <map>
#include <stdexcept>
#include <tuple>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#if defined ARCH_WIN
	#include <windows.h>
	#include <direct.h>
#else
	#include <dlfcn.h> // for dlopen
#endif
#include <dirent.h>

#include <osdialog.h>
#include <jansson.h>

#include <plugin.hpp>
#include <system.hpp>
#include <asset.hpp>
#include <string.hpp>
#include <context.hpp>
#include <plugin/callbacks.hpp>
#include <settings.hpp>


namespace rack {

namespace core {
void init(rack::plugin::Plugin* plugin);
} // namespace core

namespace plugin {


////////////////////
// private API
////////////////////


static void* getSymbol(void* handle, const char* name) {
	if (!handle)
		return NULL;

#if defined ARCH_WIN
	return (void*) GetProcAddress((HMODULE) handle, name);
#else
	return dlsym(handle, name);
#endif
}

/** Returns library handle */
static void* loadLibrary(std::string libraryPath) {
#if defined ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	std::wstring libraryFilenameW = string::UTF8toUTF16(libraryPath);
	HINSTANCE handle = LoadLibraryW(libraryFilenameW.c_str());
	SetErrorMode(0);
	if (!handle) {
		int error = GetLastError();
		throw Exception("Failed to load library %s: code %d", libraryPath.c_str(), error);
	}
#else
	// Since Rack 2, plugins on Linux/Mac link to the absolute path /tmp/Rack2/libRack.<ext>
	// Create a symlink at /tmp/Rack2 to the system dir containting libRack.
	std::string systemDir = system::getAbsolute(asset::systemDir);
	std::string linkPath = "/tmp/Rack2";
	if (!settings::devMode) {
		// Clean up old symbolic link in case a different edition was run earlier
		system::remove(linkPath);
		system::createSymbolicLink(systemDir, linkPath);
	}
	// Load library with dlopen
	void* handle = NULL;
	#if defined ARCH_LIN
	handle = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_LOCAL);
	#elif defined ARCH_MAC
	handle = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_LOCAL);
	#endif
	if (!settings::devMode) {
		system::remove(linkPath);
	}
	if (!handle)
		throw Exception("Failed to load library %s: %s", libraryPath.c_str(), dlerror());
#endif
	return handle;
}

typedef void (*InitCallback)(Plugin*);

static InitCallback loadPluginCallback(Plugin* plugin) {
	// Load plugin library
	std::string libraryExt;
#if defined ARCH_LIN
	libraryExt = "so";
#elif defined ARCH_WIN
	libraryExt = "dll";
#elif ARCH_MAC
	libraryExt = "dylib";
#endif

#if defined ARCH_X64
	// Use `plugin.EXT` on x64 for backward compatibility.
	// Change to `plugin-OS-CPU.EXT` in Rack 3.
	std::string libraryFilename = "plugin." + libraryExt;
#else
	// Use `plugin-CPU.EXT` on other CPUs like ARM64
	std::string libraryFilename = "plugin-" + APP_CPU + "." + libraryExt;
#endif
	std::string libraryPath = system::join(plugin->path, libraryFilename);

	// Check file existence
	if (!system::isFile(libraryPath))
		throw Exception("Plugin binary not found at %s", libraryPath.c_str());

	// Load dynamic/shared library
	plugin->handle = loadLibrary(libraryPath);

	// Get plugin's init() function
	InitCallback initCallback = (InitCallback) getSymbol(plugin->handle, "init");
	if (!initCallback)
		throw Exception("Failed to read init() symbol in %s", libraryPath.c_str());

	return initCallback;
}


/** If path is blank, loads Core */
static Plugin* loadPlugin(std::string path) {
	if (path == "")
		INFO("Loading Core plugin");
	else
		INFO("Loading plugin from %s", path.c_str());

	Plugin* plugin = new Plugin;
	try {
		// Set plugin path
		plugin->path = (path == "") ? asset::systemDir : path;

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
		std::string manifestFilename = (path == "") ? asset::system("Core.json") : system::join(path, "plugin.json");
		FILE* file = std::fopen(manifestFilename.c_str(), "r");
		if (!file)
			throw Exception("Manifest file %s does not exist", manifestFilename.c_str());
		DEFER({std::fclose(file);});

		json_error_t error;
		json_t* rootJ = json_loadf(file, 0, &error);
		if (!rootJ)
			throw Exception("JSON parsing error at %s %d:%d %s", manifestFilename.c_str(), error.line, error.column, error.text);
		DEFER({json_decref(rootJ);});

		// Load manifest
		plugin->fromJson(rootJ);

		// Reject plugin if slug already exists
		Plugin* existingPlugin = getPlugin(plugin->slug);
		if (existingPlugin)
			throw Exception("Plugin %s is already loaded, not attempting to load it again", plugin->slug.c_str());

		// Call init callback
		InitCallback initCallback;
		if (path == "") {
			initCallback = core::init;
		}
		else {
			initCallback = loadPluginCallback(plugin);
		}
		initCallback(plugin);

		// Load modules manifest
		json_t* modulesJ = json_object_get(rootJ, "modules");
		plugin->modulesFromJson(modulesJ);

		// Call settingsFromJson() if exists
		// Returns NULL for Core.
		auto settingsFromJson = (decltype(&::settingsFromJson)) getSymbol(plugin->handle, "settingsFromJson");
		if (settingsFromJson) {
			json_t* settingsJ = json_object_get(settings::pluginSettingsJ, plugin->slug.c_str());
			if (settingsJ)
				settingsFromJson(settingsJ);
		}
	}
	catch (Exception& e) {
		WARN("Could not load plugin %s: %s", path.c_str(), e.what());
		delete plugin;
		return NULL;
	}

	INFO("Loaded %s %s", plugin->slug.c_str(), plugin->version.c_str());
	plugins.push_back(plugin);
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
		if (!system::isFile(packagePath))
			continue;
		if (system::getExtension(packagePath) != ".vcvplugin")
			continue;

		// Extract package
		INFO("Extracting package %s", packagePath.c_str());
		try {
			system::unarchiveToDirectory(packagePath, path);
		}
		catch (Exception& e) {
			WARN("Plugin package %s failed to extract: %s", packagePath.c_str(), e.what());
			message += string::f("Could not extract plugin package %s\n", packagePath.c_str());
			continue;
		}
		// Remove package
		system::remove(packagePath.c_str());
	}
	if (!message.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
}

////////////////////
// public API
////////////////////

void init() {
	// Don't re-initialize
	assert(plugins.empty());

	// Load Core
	loadPlugin("");

	pluginsPath = asset::user("plugins");

	// Get user plugins directory
	system::createDirectory(pluginsPath);

	// Don't load plugins if safe mode is enabled
	if (settings::safeMode)
		return;

	// Extract packages and load plugins
	extractPackages(pluginsPath);
	loadPlugins(pluginsPath);

	// If Fundamental wasn't loaded, copy the bundled Fundamental package and load it
	if (!settings::devMode && !getPlugin("Fundamental")) {
		std::string fundamentalSrc = asset::system("Fundamental.vcvplugin");
		std::string fundamentalDir = system::join(pluginsPath, "Fundamental");
		if (system::isFile(fundamentalSrc)) {
			INFO("Extracting bundled Fundamental package");
			try {
				system::unarchiveToDirectory(fundamentalSrc.c_str(), pluginsPath.c_str());
				loadPlugin(fundamentalDir);
			}
			catch (Exception& e) {
				WARN("Could not extract Fundamental package: %s", e.what());
			}
		}
	}
}


static void destroyPlugin(Plugin* plugin) {
	void* handle = plugin->handle;

	// Call destroy() if defined in the plugin library
	typedef void (*DestroyCallback)();
	DestroyCallback destroyCallback = NULL;
	if (handle) {
		destroyCallback = (DestroyCallback) getSymbol(handle, "destroy");
	}
	if (destroyCallback) {
		try {
			destroyCallback();
		}
		catch (Exception& e) {
			WARN("Could not destroy plugin %s", plugin->slug.c_str());
		}
	}

	// We must delete the Plugin instance *before* freeing the library, because the vtables of Model subclasses are defined in the library, which are needed in the Plugin destructor.
	delete plugin;

	// Free library handle
	if (handle) {
#if defined ARCH_WIN
		FreeLibrary((HINSTANCE) handle);
#else
		dlclose(handle);
#endif
	}
}


void destroy() {
	while (!plugins.empty()) {
		Plugin* plugin = plugins.back();
		INFO("Destroying plugin %s", plugin->name.c_str());
		destroyPlugin(plugin);
		plugins.pop_back();
	}
	assert(plugins.empty());
}


void settingsMergeJson(json_t* rootJ) {
	for (Plugin* plugin : plugins) {
		auto settingsToJson = (decltype(&::settingsToJson)) getSymbol(plugin->handle, "settingsToJson");
		if (settingsToJson) {
			json_t* settingsJ = settingsToJson();
			json_object_set_new(rootJ, plugin->slug.c_str(), settingsJ);
		}
		else {
			json_object_del(rootJ, plugin->slug.c_str());
		}
	}
}


/** Given slug => fallback slug.
Supports bidirectional fallbacks.
To request fallback slugs to be added to this list, contact VCV support.
*/
static const std::map<std::string, std::string> pluginSlugFallbacks = {
	{"VultModulesFree", "VultModules"},
	{"VultModules", "VultModulesFree"},
	{"AudibleInstrumentsPreview", "AudibleInstruments"},
	{"SequelSequencers", "DanielDavies"},
	{"DelexanderVol1", "DelexandraVol1"},
	// {"", ""},
};


Plugin* getPlugin(const std::string& pluginSlug) {
	if (pluginSlug.empty())
		return NULL;

	auto it = std::find_if(plugins.begin(), plugins.end(), [=](Plugin* p) {
		return p->slug == pluginSlug;
	});
	if (it != plugins.end())
		return *it;
	return NULL;
}


Plugin* getPluginFallback(const std::string& pluginSlug) {
	if (pluginSlug.empty())
		return NULL;

	// Attempt example plugin
	Plugin* p = getPlugin(pluginSlug);
	if (p)
		return p;

	// Attempt fallback plugin slug
	auto it = pluginSlugFallbacks.find(pluginSlug);
	if (it != pluginSlugFallbacks.end())
		return getPlugin(it->second);

	return NULL;
}


/** Given slug => fallback slug.
Correctly handles bidirectional fallbacks.
To request fallback slugs to be added to this list, open a GitHub issue.
*/
using PluginModuleSlug = std::tuple<std::string, std::string>;
static const std::map<PluginModuleSlug, PluginModuleSlug> moduleSlugFallbacks = {
	{{"MindMeld-ShapeMasterPro", "ShapeMasterPro"}, {"MindMeldModular", "ShapeMaster"}},
	{{"MindMeldModular", "ShapeMaster"}, {"MindMeld-ShapeMasterPro", "ShapeMasterPro"}},
	// {{"", ""}, {"", ""}},
};


Model* getModel(const std::string& pluginSlug, const std::string& modelSlug) {
	if (pluginSlug.empty() || modelSlug.empty())
		return NULL;

	Plugin* p = getPlugin(pluginSlug);
	if (!p)
		return NULL;

	return p->getModel(modelSlug);
}


Model* getModelFallback(const std::string& pluginSlug, const std::string& modelSlug) {
	if (pluginSlug.empty() || modelSlug.empty())
		return NULL;

	// Attempt exact plugin and model
	Model* m = getModel(pluginSlug, modelSlug);
	if (m)
		return m;

	// Attempt fallback module
	auto it = moduleSlugFallbacks.find(std::make_tuple(pluginSlug, modelSlug));
	if (it != moduleSlugFallbacks.end()) {
		Model* m = getModel(std::get<0>(it->second), std::get<1>(it->second));
		if (m)
			return m;
	}

	// Attempt fallback plugin
	auto it2 = pluginSlugFallbacks.find(pluginSlug);
	if (it2 != pluginSlugFallbacks.end()) {
		Model* m = getModel(it2->second, modelSlug);
		if (m)
			return m;
	}

	return NULL;
}


Model* modelFromJson(json_t* moduleJ) {
	// Get slugs
	json_t* pluginSlugJ = json_object_get(moduleJ, "plugin");
	if (!pluginSlugJ)
		throw Exception("\"plugin\" property not found in module JSON");
	std::string pluginSlug = json_string_value(pluginSlugJ);
	pluginSlug = normalizeSlug(pluginSlug);

	json_t* modelSlugJ = json_object_get(moduleJ, "model");
	if (!modelSlugJ)
		throw Exception("\"model\" property not found in module JSON");
	std::string modelSlug = json_string_value(modelSlugJ);
	modelSlug = normalizeSlug(modelSlug);

	// Get Model
	Model* model = getModelFallback(pluginSlug, modelSlug);
	if (!model)
		throw Exception("Could not find module %s/%s", pluginSlug.c_str(), modelSlug.c_str());
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


std::string pluginsPath;
std::vector<Plugin*> plugins;


} // namespace plugin
} // namespace rack
