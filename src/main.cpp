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
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "util/color.hpp"

#include "osdialog.h"
#include <unistd.h>

#ifdef ARCH_WIN
	#include <Windows.h>
#endif

using namespace rack;


int main(int argc, char* argv[]) {
	bool devMode = false;
	std::string patchFile;

	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "dg:l:")) != -1) {
		switch (c) {
			case 'd': {
				devMode = true;
			} break;
			case 'g': {
				assetGlobalDir = optarg;
			} break;
			case 'l': {
				assetLocalDir = optarg;
			} break;
			default: break;
		}
	}
	if (optind < argc) {
		patchFile = argv[optind];
	}

#ifdef ARCH_WIN
	// Windows global mutex to prevent multiple instances
	// Handle will be closed by Windows when the process ends
	HANDLE instanceMutex = CreateMutex(NULL, true, gApplicationName.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Rack is already running. Multiple Rack instances are not supported.");
		exit(1);
	}
	(void) instanceMutex;
#endif

	// Initialize environment
	randomInit();
	assetInit(devMode);
	loggerInit(devMode);

	// Log environment
	info("%s %s", gApplicationName.c_str(), gApplicationVersion.c_str());
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
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipAutosaveOnLaunch = gSkipAutosaveOnLaunch;
		gSkipAutosaveOnLaunch = true;
		settingsSave(assetLocal("settings.json"));
		gSkipAutosaveOnLaunch = false;
		if (oldSkipAutosaveOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Clear your patch and start over?")) {
			gRackWidget->lastPath = "";
		}
		else {
			// Load autosave
			std::string oldLastPath = gRackWidget->lastPath;
			gRackWidget->load(assetLocal("autosave.vcv"));
			gRackWidget->lastPath = oldLastPath;
		}
	}
	else {
		// Load patch
		gRackWidget->load(patchFile);
		gRackWidget->lastPath = patchFile;
	}

	engineStart();
	windowRun();
	engineStop();

	// Destroy namespaces
	gRackWidget->save(assetLocal("autosave.vcv"));
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
