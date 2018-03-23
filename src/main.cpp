#include "util/common.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include "bridge.hpp"
#include "osdialog.h"

#include <unistd.h>

#include <dirent.h>


using namespace rack;


std::vector<std::string> filesystemListDirectory(std::string path) {
	std::vector<std::string> filenames;
	DIR *dir = opendir(path.c_str());
	if (dir) {
		struct dirent *d;
		while ((d = readdir(dir))) {
			std::string filename = d->d_name;
			if (filename == "." || filename == "..")
				continue;
			filenames.push_back(path + "/" + filename);
		}
		closedir(dir);
	}
	return filenames;
}

int main(int argc, char* argv[]) {

	for (std::string filename : filesystemListDirectory("plugins")) {
		debug("%s", filename.c_str());
	}
	return 0;

	randomInit();
	loggerInit();

	info("Rack %s", gApplicationVersion.c_str());

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
	loggerDestroy();

	return 0;
}
