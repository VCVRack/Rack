#include "common.hpp"
#include "random.hpp"
#include "logger.hpp"
#include "asset.hpp"
#include "rtmidi.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "bridge.hpp"
#include "settings.hpp"
#include "engine/Engine.hpp"
#include "app/Scene.hpp"
#include "tags.hpp"
#include "plugin/PluginManager.hpp"

#include <unistd.h>
#include <osdialog.h>

#ifdef ARCH_WIN
	#include <Windows.h>
#endif

using namespace rack;


int main(int argc, char *argv[]) {
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

	bool devMode = false;
	std::string patchFile;

	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "ds:u:")) != -1) {
		switch (c) {
			case 'd': {
				devMode = true;
			} break;
			case 's': {
				asset::systemDir = optarg;
			} break;
			case 'u': {
				asset::userDir = optarg;
			} break;
			default: break;
		}
	}
	if (optind < argc) {
		patchFile = argv[optind];
	}

	// Initialize environment
	random::init();
	asset::init(devMode);
	logger::init(devMode);

	// Log environment
	INFO("%s %s", APP_NAME.c_str(), APP_VERSION.c_str());
	if (devMode)
		INFO("Development mode");
	INFO("System directory: %s", asset::system("").c_str());
	INFO("User directory: %s", asset::user("").c_str());

	// Initialize app
	tagsInit();
	gPluginManager = new PluginManager(devMode);
	gEngine = new Engine;
	rtmidiInit();
	bridgeInit();
	keyboard::init();
	gamepad::init();
	event::gContext = new event::Context;
	gScene = new Scene;
	gScene->devMode = devMode;
	event::gContext->rootWidget = gScene;
	windowInit();
	settings::load(asset::user("settings.json"));

	if (patchFile.empty()) {
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipAutosaveOnLaunch = settings::gSkipAutosaveOnLaunch;
		settings::gSkipAutosaveOnLaunch = true;
		settings::save(asset::user("settings.json"));
		settings::gSkipAutosaveOnLaunch = false;
		if (oldSkipAutosaveOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Clear your patch and start over?")) {
			gScene->rackWidget->lastPath = "";
		}
		else {
			// Load autosave
			std::string oldLastPath = gScene->rackWidget->lastPath;
			gScene->rackWidget->load(asset::user("autosave.vcv"));
			gScene->rackWidget->lastPath = oldLastPath;
		}
	}
	else {
		// Load patch
		gScene->rackWidget->load(patchFile);
		gScene->rackWidget->lastPath = patchFile;
	}

	gEngine->start();
	windowRun();
	gEngine->stop();

	// Destroy namespaces
	gScene->rackWidget->save(asset::user("autosave.vcv"));
	settings::save(asset::user("settings.json"));
	delete gScene;
	gScene = NULL;
	delete event::gContext;
	event::gContext = NULL;
	windowDestroy();
	bridgeDestroy();
	delete gEngine;
	gEngine = NULL;
	midiDestroy();
	delete gPluginManager;
	gPluginManager = NULL;
	logger::destroy();

	return 0;
}
