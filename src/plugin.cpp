#include "global_pre.hpp"
#include "plugin.hpp"
#include "Core/Core.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "util/request.hpp"
#include "osdialog.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef YAC_POSIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#include <dirent.h>
#else
#include "util/dirent_win32/dirent.h"
#endif // YAC_POSIX

#include <thread>
#include <stdexcept>

#include "global.hpp"
#include "global_ui.hpp"

#ifndef USE_VST2
#define ZIP_STATIC
#include <zip.h>
#endif // USE_VST2
#include <jansson.h>

#if ARCH_WIN
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif


extern "C" { extern void init_plugin_Core (rack::Plugin *p); }

namespace rack {


#ifdef USE_VST2
#ifndef RACK_PLUGIN
extern void vst2_load_static_rack_plugins(void);
#endif // RACK_PLUGIN
#endif // USE_VST2


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

static bool loadPlugin(std::string path, bool _bFX) {
#ifdef RACK_HOST
	std::string libraryFilename;
#if ARCH_LIN
	libraryFilename = path + "/" + "plugin.so" + (_bFX ? ".fx" : ".instr");
#elif ARCH_WIN
	libraryFilename = path + "/" + "plugin.dll" + (_bFX ? ".fx" : ".instr");
#elif ARCH_MAC
	libraryFilename = path + "/" + "plugin.dylib" + (_bFX ? ".fx" : ".instr");
#endif

	// Check file existence
	if (!systemIsFile(libraryFilename)) {
		// warn("Plugin file %s does not exist", libraryFilename.c_str());
		return false;
	}
   else {
      info("trying to load shared plugin file %s", libraryFilename.c_str());
   }

	// Load dynamic/shared library
#if ARCH_WIN
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
// #ifdef USE_VST2
//    std::wstring libNameW = std::wstring(libraryFilename.begin(), libraryFilename.end());
//    HINSTANCE handle = LoadLibraryW(libNameW.c_str());
 
// #else
	HINSTANCE handle = LoadLibrary(libraryFilename.c_str());
// #endif
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
	InitCallback initCallback;
#ifdef USE_VST2
#define init_symbol_name "init_plugin"
#else
#define init_symbol_name "init"
#endif // USE_VST2
#if ARCH_WIN
	initCallback = (InitCallback) GetProcAddress(handle, init_symbol_name);
#else
	initCallback = (InitCallback) dlsym(handle, init_symbol_name);
#endif
	if (!initCallback) {
		warn("Failed to read init() symbol in %s", libraryFilename.c_str());
		return false;
	}

	// Construct and initialize Plugin instance
	Plugin *plugin = new Plugin();
	plugin->path = path;
	plugin->handle = handle;
#ifdef USE_VST2
#ifdef RACK_HOST
   plugin->vst2_handle_ui_param_fxn = &vst2_handle_ui_param;
   plugin->vst2_queue_param_sync_fxn = &vst2_queue_param_sync;
   plugin->global = global;
   plugin->global_ui = global_ui;
   printf("xxx vstrack_plugin: loadPlugin: global=%p plugin->global=%p\n", global, plugin->global);
#endif // RACK_HOST
#endif // USE_VST2
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
	global->plugin.gPlugins.push_back(plugin);
	info("Loaded plugin %s %s from %s", plugin->slug.c_str(), plugin->version.c_str(), libraryFilename.c_str());

#else
   (void)path;
#endif // RACK_HOST

	return true;
}

#ifndef USE_VST2
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
	downloadUrl = global_ui->app.gApiHost;
	downloadUrl += "/download";
	if (dryRun) {
		downloadUrl += "/available";
	}
	downloadUrl += "?token=" + requestEscape(global->plugin.gToken);
	downloadUrl += "&slug=" + requestEscape(slug);
	downloadUrl += "&version=" + requestEscape(latestVersion);
	downloadUrl += "&arch=" + requestEscape(arch);
#ifdef USE_VST2
	downloadUrl += "&format=vst2";
#endif // USE_VST2

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
		global->plugin.downloadName = name;
		global->plugin.downloadProgress = 0.0;
		info("Downloading plugin %s %s %s", slug.c_str(), latestVersion.c_str(), arch.c_str());

		// Download zip
		std::string pluginDest = assetLocal("plugins/" + slug + ".zip");
		if (!requestDownload(downloadUrl, pluginDest, &global->plugin.downloadProgress)) {
			warn("Plugin %s download was unsuccessful", slug.c_str());
			return false;
		}

		global->plugin.downloadName = "";
		return true;
	}
}
#endif // USE_VST2

static void loadPlugins(std::string path, bool _bFX) {
#ifdef RACK_HOST
	std::string message;
	for (std::string pluginPath : systemListEntries(path)) {
		if (!systemIsDirectory(pluginPath))
			continue;
		if (!loadPlugin(pluginPath, _bFX)) {
#ifndef USE_VST2
         // (note) skip message (some plugins are linked statically in VST2 build)
			message += stringf("Could not load plugin %s\n", pluginPath.c_str());
#endif // USE_VST2
		}
	}
	if (!message.empty()) {
		message += "See log for details.";
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
#else
   (void)path;
#endif // RACK_HOST
}

#ifndef USE_VST2
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
#endif // USE_VST2

#ifndef USE_VST2
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
#endif // USE_VST2

#ifndef USE_VST2
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
#endif // USE_VST2

////////////////////
// public API
////////////////////

void pluginInit(bool devMode, bool _bFX) {
	tagsInit();
#ifdef RACK_HOST

	// Load core
	// This function is defined in core.cpp
	Plugin *corePlugin = new Plugin();
	init_plugin_Core(corePlugin);
	global->plugin.gPlugins.push_back(corePlugin);

   // Init statically linked plugins
   vst2_load_static_rack_plugins();

	// Get local plugins directory
	std::string localPlugins = assetLocal("plugins");
	mkdir(localPlugins.c_str(), 0755);

#ifndef USE_VST2
	if (!devMode) {
		// Copy Fundamental package to plugins directory if folder does not exist
		std::string fundamentalSrc = assetGlobal("Fundamental.zip");
		std::string fundamentalDest = assetLocal("plugins/Fundamental.zip");
		std::string fundamentalDir = assetLocal("plugins/Fundamental");
		if (systemIsFile(fundamentalSrc) && !systemIsFile(fundamentalDest) && !systemIsDirectory(fundamentalDir)) {
			systemCopy(fundamentalSrc, fundamentalDest);
		}
	}
#endif // USE_VST2

	// Extract packages and load plugins
#ifndef USE_VST2
	extractPackages(localPlugins);
#endif // USE_VST2

   // Load/init dynamically loaded plugins
	loadPlugins(localPlugins, _bFX);

#endif // RACK_HOST
}

void pluginDestroy() {
#ifndef USE_VST2
	for (Plugin *plugin : global->plugin.gPlugins) {
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
#endif // USE_VST2
	global->plugin.gPlugins.clear();
}

bool pluginSync(bool dryRun) {
#ifndef USE_VST2
	if (global->plugin.gToken.empty())
		return false;

	bool available = false;

	if (!dryRun) {
		global->plugin.isDownloading = true;
		global->plugin.downloadProgress = 0.0;
		global->plugin.downloadName = "Updating plugins...";
	}
	defer({
      global->plugin.isDownloading = false;
	});

	// Get user's plugins list
	json_t *pluginsReqJ = json_object();
	json_object_set(pluginsReqJ, "token", json_string(global->plugin.gToken.c_str()));
	json_t *pluginsResJ = requestJson(METHOD_GET, global_ui->app.gApiHost + "/plugins", pluginsReqJ);
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
	json_t *manifestsResJ = requestJson(METHOD_GET, global_ui->app.gApiHost + "/community/manifests", NULL);
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
#else
   return false;
#endif // USE_VST2
}

void pluginLogIn(std::string email, std::string password) {
#ifndef USE_VST2
	json_t *reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	json_t *resJ = requestJson(METHOD_POST, global_ui->app.gApiHost + "/token", reqJ);
	json_decref(reqJ);

	if (resJ) {
		json_t *errorJ = json_object_get(resJ, "error");
		if (errorJ) {
			const char *errorStr = json_string_value(errorJ);
			global->plugin.loginStatus = errorStr;
		}
		else {
			json_t *tokenJ = json_object_get(resJ, "token");
			if (tokenJ) {
				const char *tokenStr = json_string_value(tokenJ);
				global->plugin.gToken = tokenStr;
				global->plugin.loginStatus = "";
			}
		}
		json_decref(resJ);
	}
#endif
}

void pluginLogOut() {
	global->plugin.gToken = "";
}

void pluginCancelDownload() {
	// TODO
}

bool pluginIsLoggedIn() {
	return global->plugin.gToken != "";
}

bool pluginIsDownloading() {
	return global->plugin.isDownloading;
}

float pluginGetDownloadProgress() {
	return global->plugin.downloadProgress;
}

std::string pluginGetDownloadName() {
	return global->plugin.downloadName;
}

std::string pluginGetLoginStatus() {
	return global->plugin.loginStatus;
}

Plugin *pluginGetPlugin(std::string pluginSlug) {
	for (Plugin *plugin : global->plugin.gPlugins) {
      // printf("xxx pluginGetPlugin: find pluginSlug=\"%s\" current=\"%s\"\n", pluginSlug.c_str(), plugin->slug.c_str());
		if (plugin->slug == pluginSlug) {
			return plugin;
		}
	}
	return NULL;
}

Model *pluginGetModel(std::string pluginSlug, std::string modelSlug) {
	Plugin *plugin = pluginGetPlugin(pluginSlug);
   // printf("xxx vstrack_plugin: plugSlug=\"%s\" modelSlug=\"%s\" => plugin=%p\n", pluginSlug.c_str(), modelSlug.c_str(), plugin);
	if (plugin) {
		for (Model *model : plugin->models) {
			if (model->slug == modelSlug) {
            // printf("xxx vstrack_plugin: plugSlug=\"%s\" modelSlug=\"%s\" => plugin=%p model=%p\n", pluginSlug.c_str(), modelSlug.c_str(), plugin, model);
				return model;
			}
		}
	}
	return NULL;
}


} // namespace rack

using namespace rack;

#ifdef USE_VST2
#ifdef ARCH_WIN
extern "C" { extern long seed_initialized; }
#else
extern "C" { extern volatile char seed_initialized; }
#endif // ARCH_WIN
extern "C" { extern volatile uint32_t hashtable_seed; }
void vst2_set_shared_plugin_tls_globals(void) {
   // Called in audio thread (see vst2_main.cpp:VSTPluginProcessReplacingFloat32())
   for(Plugin *p : global->plugin.gPlugins) {
      if(NULL != p->set_tls_globals_fxn) {
         if(0)
         {
            static int xxx = 0;
            if(0 == (++xxx & 255))
               printf("xxx vstrack_plugin: calling p->set_tls_globals_fxn() global=%p p->global=%p\n", global, p->global);
         }
         p->json.hashtable_seed = hashtable_seed;
         p->json.seed_initialized = seed_initialized;
         p->set_tls_globals_fxn(p);
         // printf("xxx vstrack_plugin: calling p->set_tls_globals_fxn() OK\n");
      }
   }
}
#endif // USE_VST2

RackScene *rack_plugin_ui_get_rackscene(void) {
#ifdef USE_VST2
   return rack::global_ui->app.gRackScene;
#else
   return gRackScene;
#endif // USE_VST2
}

RackWidget *rack_plugin_ui_get_rackwidget(void) {
#ifdef USE_VST2
   return rack::global_ui->app.gRackWidget;
#else
   return gRackWidget;
#endif // USE_VST2
}

Toolbar *rack_plugin_ui_get_toolbar(void) {
#ifdef USE_VST2
   return rack::global_ui->app.gToolbar;
#else
   return gToolbar;
#endif // USE_VST2
}

Widget *rack_plugin_ui_get_hovered_widget(void) {
#ifdef USE_VST2
   return rack::global_ui->widgets.gHoveredWidget;
#else
   return gHoveredWidget;
#endif // USE_VST2
}

Widget *rack_plugin_ui_get_dragged_widget(void) {
#ifdef USE_VST2
   return rack::global_ui->widgets.gDraggedWidget;
#else
   return gDraggedWidget;
#endif // USE_VST2
}

void rack_plugin_ui_set_dragged_widget(Widget *w) {
#ifdef USE_VST2
   rack::global_ui->widgets.gDraggedWidget = w;
#else
   gDraggedWidget = w;
#endif // USE_VST2
}

Widget *rack_plugin_ui_get_draghovered_widget(void) {
#ifdef USE_VST2
   return rack::global_ui->widgets.gDragHoveredWidget;
#else
   return gDragHovered;
#endif // USE_VST2
}

Widget *rack_plugin_ui_get_focused_widget(void) {
#ifdef USE_VST2
   return rack::global_ui->widgets.gFocusedWidget;
#else
   return gFocusedWidget;
#endif // USE_VST2
}

void rack_plugin_ui_set_focused_widget(Widget *w) {
#ifdef USE_VST2
   rack::global_ui->widgets.gFocusedWidget = w;
#else
   gFocusedWidget = w;
#endif // USE_VST2
}
