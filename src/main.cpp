#include "common.hpp"
#include "random.hpp"
#include "asset.hpp"
#include "midi.hpp"
#include "rtmidi.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "bridge.hpp"
#include "settings.hpp"
#include "engine/Engine.hpp"
#include "app/Scene.hpp"
#include "plugin.hpp"
#include "app.hpp"
#include "patch.hpp"
#include "ui.hpp"

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
	HANDLE instanceMutex = CreateMutex(NULL, true, APP_NAME.c_str());
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
	asset::init(devMode);
	logger::init(devMode);

	// Log environment
	INFO("%s %s", APP_NAME.c_str(), APP_VERSION.c_str());
	if (devMode)
		INFO("Development mode");
	INFO("System directory: %s", asset::systemDir.c_str());
	INFO("User directory: %s", asset::userDir.c_str());

	random::init();
	midi::init();
	rtmidiInit();
	bridgeInit();
	keyboard::init();
	gamepad::init();
	ui::init();
	plugin::init(devMode);
	windowInit();
	INFO("Initialized environment");

	// Initialize app
	appInit();
	app()->scene->devMode = devMode;
	settings::load(asset::user("settings.json"));

	if (patchFile.empty()) {
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipLoadOnLaunch = settings::skipLoadOnLaunch;
		settings::skipLoadOnLaunch = true;
		settings::save(asset::user("settings.json"));
		settings::skipLoadOnLaunch = false;
		if (oldSkipLoadOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Clear your patch and start over?")) {
			app()->patch->path = "";
		}
		else {
			// Load autosave
			std::string oldLastPath = app()->patch->path;
			app()->patch->load(asset::user("autosave.vcv"));
			app()->patch->path = oldLastPath;
		}
	}
	else {
		// Load patch
		app()->patch->load(patchFile);
		app()->patch->path = patchFile;
	}
	INFO("Initialized app");

	app()->engine->start();
	app()->window->run();
	INFO("Window closed");
	app()->engine->stop();

	// Destroy app
	app()->patch->save(asset::user("autosave.vcv"));
	settings::save(asset::user("settings.json"));
	appDestroy();
	INFO("Cleaned up app");

	// Destroy environment
	windowDestroy();
	plugin::destroy();
	ui::destroy();
	bridgeDestroy();
	midi::destroy();
	INFO("Cleaned up environment");
	logger::destroy();

	return 0;
}
