#include "global_pre.hpp"
#include "util/common.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include "bridge.hpp"
#include "midi.hpp"
#include "rtmidi.hpp"
#include "vstmidi.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "util/color.hpp"

#include "osdialog.h"
#ifdef YAC_POSIX
#include <unistd.h>
#endif
#include "global.hpp"
#include "global_ui.hpp"


using namespace rack;

YAC_TLS rack::Global *rack::global;
YAC_TLS rack::GlobalUI *rack::global_ui;


#ifdef USE_VST2
int vst2_init(int argc, char* argv[], bool _bFX) {
#ifdef YAC_LINUX
	bool devMode = true; // true=log to stderr, false=log to file
#else
	bool devMode = false;
#endif // YAC_LINUX
	std::string patchFile;

#if 0
	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "d")) != -1) {
		switch (c) {
			case 'd': {
				devMode = true;
			} break;
			default: break;
		}
	}
	if (optind < argc) {
		patchFile = argv[optind];
	}
#endif

   printf("xxx vst2_init: 1\n");
	// Initialize environment
	randomInit();
   printf("xxx vst2_init: 2\n");
	assetInit(devMode);
   printf("xxx vst2_init: 3\n");
	loggerInit(devMode);
   printf("xxx vst2_init: 4 global_ui=%p\n", global_ui);

	// Log environment
	info("%s %s", global_ui->app.gApplicationName.c_str(), global_ui->app.gApplicationVersion.c_str());
	if (devMode)
		info("Development mode");
	info("Global directory: %s", assetGlobal("").c_str());
	info("Local directory: %s", assetLocal("").c_str());

   printf("xxx vst2_init: 5\n");

	// Initialize app
	pluginInit(devMode, _bFX);
   printf("xxx vst2_init: 6\n");
	engineInit();
   printf("xxx vst2_init: 7\n");
#ifndef USE_VST2
	rtmidiInit();
	bridgeInit();
#endif // USE_VST2
   vstmidiInit();
   printf("xxx vst2_init: 8\n");
#ifndef USE_VST2
	keyboardInit();
	gamepadInit();
#endif // USE_VST2
	windowInit();
   printf("xxx vst2_init: 9\n");
	appInit(devMode);
   printf("xxx vst2_init: 10\n");
   if(!settingsLoad(assetLocal("settings.json"), false/*bWindowSizeOnly*/))
      settingsLoad(assetLocal("settings_default.json"), false/*bWindowSizeOnly*/);
   printf("xxx vst2_init: 11\n");

#if 0
	if (patchFile.empty()) {
		std::string oldLastPath = global_ui->app.gRackWidget->lastPath;
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipAutosaveOnLaunch = global->settings.gSkipAutosaveOnLaunch;
		global->settings.gSkipAutosaveOnLaunch = true;
		settingsSave(assetLocal("settings.json"));
		global->settings.gSkipAutosaveOnLaunch = false;
		if (oldSkipAutosaveOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Would you like to clear your patch and start over?")) {
			// Do nothing. Empty patch is already loaded.
		}
		else {
			// Load autosave
			global_ui->app.gRackWidget->loadPatch(assetLocal("autosave.vcv"));
		}
		global_ui->app.gRackWidget->lastPath = oldLastPath;
	}
	else {
		// Load patch
		global_ui->app.gRackWidget->loadPatch(patchFile);
	}
#endif

	// // engineStart();  // starts bg thread for audio rendering
   printf("xxx vst2_init: LEAVE\n");
   return 0;
}

void vst2_exit(void) {
	// Destroy namespaces
	// // engineStop();
   printf("xxx vst2_exit 1\n");
	// global_ui->app.gRackWidget->savePatch(assetLocal("autosave.vcv"));
	// settingsSave(assetLocal("settings.json"));
   printf("xxx vst2_exit 2\n");

#if 1
   lglw_glcontext_push(global_ui->window.lglw);
	appDestroy();
   lglw_glcontext_pop(global_ui->window.lglw);
#endif

   printf("xxx vst2_exit 3\n");
   windowDestroy();

#if 0
#ifndef USE_VST2
	bridgeDestroy();
#endif // USE_VST2
   printf("xxx vst2_exit 4\n");
	engineDestroy();
   printf("xxx vst2_exit 5\n");
#endif

   printf("xxx vst2_exit destroy midi\n");
	midiDestroy();
   printf("xxx vst2_exit destroy midi done\n");

#if 1
   printf("xxx vst2_exit 6\n");
	pluginDestroy();
   printf("xxx vst2_exit 7\n");
	loggerDestroy();
#endif

   printf("xxx vst2_exit 8 (leave)\n");
}


#else

static rack::Global loc_global;
static rack::GlobalUI loc_global_ui;

int main(int argc, char* argv[]) {
	bool devMode = false;
	std::string patchFile;

   loc_global.init();
   loc_global_ui.init();
   rack::global = &loc_global;
   rack::global_ui = &loc_global_ui;

	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "d")) != -1) {
		switch (c) {
			case 'd': {
				devMode = true;
			} break;
			default: break;
		}
	}
	if (optind < argc) {
		patchFile = argv[optind];
	}

	// Initialize environment
	randomInit();
	assetInit(devMode);
	loggerInit(devMode);

	// Log environment
	info("%s %s", global_ui->app.gApplicationName.c_str(), global_ui->app.gApplicationVersion.c_str());
	if (devMode)
		info("Development mode");
	info("Global directory: %s", assetGlobal("").c_str());
	info("Local directory: %s", assetLocal("").c_str());

	// Initialize app
	pluginInit(devMode);
	engineInit();
	rtmidiInit();
	bridgeInit();
	keyboardInit();
	gamepadInit();
	windowInit();
	appInit(devMode);
	settingsLoad(assetLocal("settings.json"));

	if (patchFile.empty()) {
		std::string oldLastPath = global_ui->app.gRackWidget->lastPath;
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipAutosaveOnLaunch = global->settings.gSkipAutosaveOnLaunch;
		global->settings.gSkipAutosaveOnLaunch = true;
		settingsSave(assetLocal("settings.json"));
		global->settings.gSkipAutosaveOnLaunch = false;
		if (oldSkipAutosaveOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Would you like to clear your patch and start over?")) {
			// Do nothing. Empty patch is already loaded.
		}
		else {
			// Load autosave
			global_ui->app.gRackWidget->loadPatch(assetLocal("autosave.vcv"));
		}
		global_ui->app.gRackWidget->lastPath = oldLastPath;
	}
	else {
		// Load patch
		global_ui->app.gRackWidget->loadPatch(patchFile);
	}

	engineStart();
	windowRun();
	engineStop();

	// Destroy namespaces
	global_ui->app.gRackWidget->savePatch(assetLocal("autosave.vcv"));
	settingsSave(assetLocal("settings.json"));
	appDestroy();
	windowDestroy();
	bridgeDestroy();
	engineDestroy();
	midiDestroy();
	pluginDestroy();
	loggerDestroy();

	return 0;
}
#endif // USE_VST2
