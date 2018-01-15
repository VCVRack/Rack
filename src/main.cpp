#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <unistd.h>


using namespace rack;

int main(int argc, char* argv[]) {
	randomSeedTime();

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
	guiInit();
	sceneInit();
	settingsLoad(assetLocal("settings.json"));

	// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
	bool oldSkipAutosaveOnLaunch = skipAutosaveOnLaunch;
	skipAutosaveOnLaunch = true;
	settingsSave(assetLocal("settings.json"));
	skipAutosaveOnLaunch = false;
	if (!oldSkipAutosaveOnLaunch)
		gRackWidget->loadPatch(assetLocal("autosave.vcv"));

	engineStart();
	guiRun();
	engineStop();

	gRackWidget->savePatch(assetLocal("autosave.vcv"));
	settingsSave(assetLocal("settings.json"));
	sceneDestroy();
	guiDestroy();
	engineDestroy();
	pluginDestroy();

#ifdef RELEASE
	fclose(gLogFile);
#endif

	return 0;
}
