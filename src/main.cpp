#include "util/common.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include "bridge.hpp"
#include <unistd.h>
#include "osdialog.h"


using namespace rack;

int main(int argc, char* argv[]) {
	randomInit();

#ifdef RELEASE
	std::string logFilename = assetLocal("log.txt");
	gLogFile = fopen(logFilename.c_str(), "w");
#endif

	info("Rack v%s", gApplicationVersion.c_str());

	{
		char *cwd = getcwd(NULL, 0);
		info("Current working directory: %s", cwd);
		free(cwd);
		std::string globalDir = assetGlobal("");
		std::string localDir = assetLocal("");
		info("Global directory: %s", globalDir.c_str());
		info("Local directory: %s", localDir.c_str());
	}

	pluginInit();
	engineInit();
	bridgeInit();
	windowInit();
	appInit();
	settingsLoad(assetLocal("settings.json"));
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
		gRackWidget->loadPatch(assetLocal("autosave.vcv"));
	}
	gRackWidget->lastPath = oldLastPath;

	engineStart();
	windowRun();
	engineStop();

	gRackWidget->savePatch(assetLocal("autosave.vcv"));
	settingsSave(assetLocal("settings.json"));
	appDestroy();
	windowDestroy();
	bridgeDestroy();
	engineDestroy();
	pluginDestroy();

#ifdef RELEASE
	fclose(gLogFile);
#endif

	return 0;
}
