#include "util/common.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include "bridge.hpp"
#include "midi.hpp"
#include "osdialog.h"

#include <unistd.h>


using namespace rack;


int main(int argc, char* argv[]) {
	randomInit();
	loggerInit();

	// Parse command line arguments
	std::string patchFile;
	if (argc >= 2) {
		patchFile = argv[1];
	}

	info("Rack %s", gApplicationVersion.c_str());

	// Log directories
	info("Global directory: %s", assetGlobal("").c_str());
	info("Local directory: %s", assetLocal("").c_str());

	// Initialize namespaces
	pluginInit();
	engineInit();
	midiInit();
	bridgeInit();
	windowInit();
	appInit();
	settingsLoad(assetLocal("settings.json"));

	if (patchFile.empty()) {
		std::string oldLastPath = gRackWidget->lastPath;
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipAutosaveOnLaunch = skipAutosaveOnLaunch;
		skipAutosaveOnLaunch = true;
		settingsSave(assetLocal("settings.json"));
		skipAutosaveOnLaunch = false;
		if (oldSkipAutosaveOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Would you like to clear your patch and start over?")) {
			// Do nothing. Empty patch is already loaded.
		}
		else {
			// Load autosave
			gRackWidget->loadPatch(assetLocal("autosave.vcv"));
		}
		gRackWidget->lastPath = oldLastPath;
	}
	else {
		// Load patch
		gRackWidget->loadPatch(patchFile);
	}

	engineStart();
	windowRun();
	engineStop();

	// Destroy namespaces
	gRackWidget->savePatch(assetLocal("autosave.vcv"));
	settingsSave(assetLocal("settings.json"));
	appDestroy();
	windowDestroy();
	bridgeDestroy();
	engineDestroy();
	pluginDestroy();
	loggerDestroy();

	return 0;
}
