#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"
#include "asset.hpp"


using namespace rack;

int main(int argc, char* argv[]) {
	pluginInit();
	engineInit();
	guiInit();
	sceneInit();
	settingsLoad(assetLocal("settings.json"));
	if (argc >= 2)
		gRackWidget->loadPatch(argv[1]);
	else
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
	return 0;
}
