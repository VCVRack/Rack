#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <unistd.h>


using namespace rack;

int main(int argc, char* argv[]) {
	char *cwd = getcwd(NULL, 0);
	printf("Current working directory is %s\n", cwd);
	free(cwd);

	pluginInit();
	engineInit();
	guiInit();
	sceneInit();
	if (argc >= 2) {
		// TODO Set gRackWidget->lastPath
		gRackWidget->loadPatch(argv[1]);
	}
	else {
		gRackWidget->loadPatch(assetLocal("autosave.vcv"));
	}
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
	return 0;
}
