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

#ifdef VERSION
	std::string logFilename = assetLocal("log.txt");
	gLogFile = fopen(logFilename.c_str(), "w");
#endif

	if (!gApplicationVersion.empty()) {
		info("Rack v%s", gApplicationVersion.c_str());
	}

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
	gRackWidget->loadPatch(assetLocal("autosave.vcv"));
	settingsLoad(assetLocal("settings.json"));

	engineStart();
	guiRun();
	engineStop();

	settingsSave(assetLocal("settings.json"));
	gRackWidget->savePatch(assetLocal("autosave.vcv"));
	sceneDestroy();
	guiDestroy();
	engineDestroy();
	pluginDestroy();

#ifdef VERSION
	fclose(gLogFile);
#endif

	return 0;
}
