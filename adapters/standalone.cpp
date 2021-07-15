#include <common.hpp>
#include <random.hpp>
#include <asset.hpp>
#include <audio.hpp>
#include <rtaudio.hpp>
#include <midi.hpp>
#include <rtmidi.hpp>
#include <keyboard.hpp>
#include <gamepad.hpp>
#include <settings.hpp>
#include <engine/Engine.hpp>
#include <app/common.hpp>
#include <app/Scene.hpp>
#include <plugin.hpp>
#include <context.hpp>
#include <Window.hpp>
#include <patch.hpp>
#include <history.hpp>
#include <ui/common.hpp>
#include <system.hpp>
#include <string.hpp>
#include <library.hpp>
#include <network.hpp>

#include <osdialog.h>
#include <thread>
#include <unistd.h> // for getopt
#include <signal.h> // for signal
#if defined ARCH_WIN
	#include <windows.h> // for CreateMutex
#endif

#if defined ARCH_MAC
	#define GLFW_EXPOSE_NATIVE_COCOA
	#include <GLFW/glfw3native.h> // for glfwGetOpenedFilenames()
#endif


using namespace rack;


static void fatalSignalHandler(int sig) {
	// Ignore this signal to avoid recursion.
	signal(sig, NULL);
	// Ignore abort() since we call it below.
	signal(SIGABRT, NULL);

	FATAL("Fatal signal %d. Stack trace:\n%s", sig, system::getStackTrace().c_str());

	// This might fail because we might not be in the main thread.
	// But oh well, we're crashing anyway.
	std::string text = APP_NAME + " has crashed. See " + logger::logPath + " for details.";
	osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, text.c_str());

	abort();
}


// TODO Use -municode on Windows and change this to wmain. Convert argv to UTF-8.
int main(int argc, char* argv[]) {
#if defined ARCH_WIN
	// Windows global mutex to prevent multiple instances
	// Handle will be closed by Windows when the process ends
	HANDLE instanceMutex = CreateMutexW(NULL, true, string::UTF8toUTF16(APP_NAME).c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Rack is already running. Multiple Rack instances are not supported.");
		exit(1);
	}
	(void) instanceMutex;

	// Don't display "Assertion failed!" dialog message.
	_set_error_mode(_OUT_TO_STDERR);
#endif

	std::string patchPath;
	bool screenshot = false;
	float screenshotZoom = 1.f;

	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "dht:s:u:p:")) != -1) {
		switch (c) {
			case 'd': {
				settings::devMode = true;
			} break;
			case 'h': {
				settings::headless = true;
			} break;
			case 't': {
				screenshot = true;
				std::sscanf(optarg, "%f", &screenshotZoom);
			} break;
			case 's': {
				asset::systemDir = optarg;
			} break;
			case 'u': {
				asset::userDir = optarg;
			} break;
			// Mac "app translocation" passes a nonsense -psn_... flag, so -p is reserved.
			case 'p': break;
			default: break;
		}
	}
	if (optind < argc) {
		patchPath = argv[optind];
	}

	// Initialize environment
	system::init();
	asset::init();
	bool loggerWasTruncated = logger::isTruncated();
	logger::init();
	random::init();

	// Test code
	// exit(0);

	// We can now install a signal handler and log the output
	if (!settings::devMode) {
		signal(SIGABRT, fatalSignalHandler);
		signal(SIGFPE, fatalSignalHandler);
		signal(SIGILL, fatalSignalHandler);
		signal(SIGSEGV, fatalSignalHandler);
		signal(SIGTERM, fatalSignalHandler);
	}

	// Log environment
	INFO("%s %s v%s", APP_NAME.c_str(), APP_VARIANT.c_str(), APP_VERSION.c_str());
	INFO("%s", system::getOperatingSystemInfo().c_str());
	std::string argsList;
	for (int i = 0; i < argc; i++) {
		argsList += argv[i];
		argsList += " ";
	}
	INFO("Args: %s", argsList.c_str());
	if (settings::devMode)
		INFO("Development mode");
	INFO("System directory: %s", asset::systemDir.c_str());
	INFO("User directory: %s", asset::userDir.c_str());
#if defined ARCH_MAC
	INFO("Bundle path: %s", asset::bundlePath.c_str());
#endif

	// Load settings
	settings::init();
	try {
		settings::load();
	}
	catch (Exception& e) {
		std::string msg = e.what();
		msg += "\n\nReset settings to default?";
		if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, msg.c_str())) {
			exit(1);
		}
	}

	// Check existence of the system res/ directory
	std::string resDir = asset::system("res");
	if (!system::isDir(resDir)) {
		std::string message = string::f("Rack's resource directory \"%s\" does not exist. Make sure Rack is correctly installed and launched.", resDir.c_str());
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, message.c_str());
		exit(1);
	}

	INFO("Initializing environment");
	network::init();
	audio::init();
	rtaudioInit();
	midi::init();
	rtmidiInit();
	keyboard::init();
	gamepad::init();
	plugin::init();
	library::init();
	if (!settings::headless) {
		ui::init();
		windowInit();
	}

	// Initialize context
	INFO("Initializing context");
	contextSet(new Context);
	APP->engine = new engine::Engine;
	APP->history = new history::State;
	APP->event = new widget::EventState;
	APP->scene = new app::Scene;
	APP->event->rootWidget = APP->scene;
	APP->patch = new PatchManager;
	if (!settings::headless) {
		APP->window = new Window;
	}

	// On Mac, use a hacked-in GLFW addition to get the launched path.
#if defined ARCH_MAC
	// For some reason, launching from the command line sets glfwGetOpenedFilenames(), so make sure we're running the app bundle.
	if (asset::bundlePath != "") {
		// const char* const* openedFilenames = glfwGetOpenedFilenames();
		// if (openedFilenames && openedFilenames[0]) {
		// 	patchPath = openedFilenames[0];
		// }
	}
#endif

	// Initialize patch
	if (loggerWasTruncated && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack crashed during the last session, possibly due to a buggy module in your patch. Clear your patch and start over?")) {
		// Do nothing
	}
	else {
		APP->patch->launch(patchPath);
	}

	// Run context
	if (settings::headless) {
		printf("Press enter to exit.\n");
		getchar();
	}
	else if (screenshot) {
		INFO("Taking screenshots of all modules at %gx zoom", screenshotZoom);
		APP->window->screenshotModules(asset::user("screenshots"), screenshotZoom);
	}
	else {
		INFO("Running window");
		APP->window->run();
		INFO("Stopped window");

		// INFO("Destroying window");
		// delete APP->window;
		// APP->window = NULL;
		// INFO("Re-creating window");
		// APP->window = new Window;
		// APP->window->run();
	}

	// Destroy context
	if (!settings::headless) {
		APP->patch->saveAutosave();
	}
	INFO("Destroying context");
	delete APP;
	contextSet(NULL);
	if (!settings::headless) {
		settings::save();
	}

	// Destroy environment
	INFO("Destroying environment");
	if (!settings::headless) {
		windowDestroy();
		ui::destroy();
	}
	library::destroy();
	midi::destroy();
	audio::destroy();
	plugin::destroy();
	INFO("Destroying logger");
	logger::destroy();

	return 0;
}
