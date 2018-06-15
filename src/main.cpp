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
#include "argagg.hpp"

#include <unistd.h>


using namespace rack;


int main(int argc, char* argv[]) {
	bool devMode = false;
	std::string patchFile;
	std::string customLocalDir;
	std::string customGlobalDir;
	int customBridgePort = -1;

	// Parse command line arguments
	argagg::parser argparser {{
		{ "help", {"-h", "--help"}, "shows this help message", 0},
		{ "devmod", {"-d", "--devmod"}, "enables dev mode (will default local/global folders to current folder)", 0},
		{ "global", {"-g", "--globaldir"}, "set golbalDir", 1},
		{ "local", {"-l", "--localdir"}, "set localDir", 1},
		{ "port", {"-p", "--port"}, "Bridge port number", 1},
	}};

	argagg::parser_results args;

	try {
		args = argparser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << "Encountered exception while parsing arguments: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (args["help"]) {
		std::cerr << "Usage: program [options] [FILENAME]" << std::endl;
		std::cerr << argparser;
		return EXIT_SUCCESS;
	}

	if (args["devmod"]) {
		devMode = true;
	}

	if (args["global"]) {
		customGlobalDir = args["global"].as<std::string>();
	}

	if (args["local"]) {
		customLocalDir = args["local"].as<std::string>();
	}

	if (args["port"]) {
		customBridgePort = args["port"].as<int>();
	}

	// Filename as first positional argument
	if (args.pos.size() > 0) {
		patchFile = args.as<std::string>(0);
	}

	// Initialize environment
	randomInit();
	assetInit(devMode, customGlobalDir, customLocalDir);
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
	if (customBridgePort > 0) {
		bridgeInit(customBridgePort);
	}
	else {
		bridgeInit();
	}
	keyboardInit();
	gamepadInit();
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
	midiDestroy();
	pluginDestroy();
	loggerDestroy();

	return 0;
}
