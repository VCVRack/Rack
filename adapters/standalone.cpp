#include <common.hpp>
#include <random.hpp>
#include <asset.hpp>
#include <audio.hpp>
#include <rtaudio.hpp>
#include <midi.hpp>
#include <rtmidi.hpp>
#include <keyboard.hpp>
#include <gamepad.hpp>
#include <midiloopback.hpp>
#include <settings.hpp>
#include <engine/Engine.hpp>
#include <app/common.hpp>
#include <app/Scene.hpp>
#include <app/Browser.hpp>
#include <plugin.hpp>
#include <context.hpp>
#include <window/Window.hpp>
#include <patch.hpp>
#include <history.hpp>
#include <ui/common.hpp>
#include <system.hpp>
#include <string.hpp>
#include <library.hpp>
#include <network.hpp>

#include <getopt.h>
#include <unistd.h> // for getopt
#include <signal.h> // for signal
#if defined ARCH_WIN
	#include <windows.h> // for CreateMutex
#endif
#include <osdialog.h>

#if defined ARCH_MAC
	#define GLFW_EXPOSE_NATIVE_COCOA
	#include <GLFW/glfw3native.h> // for glfwGetOpenedFilenames()
#endif


using namespace rack;


static void fatalSignalHandler(int sig) {
	// Ignore this signal to avoid recursion.
	signal(sig, NULL);

	std::string stackTrace = system::getStackTrace();
	FATAL("Fatal signal %d. Stack trace:\n%s", sig, stackTrace.c_str());

	// Re-raise signal
	raise(sig);
}


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
	const std::string appInfo = APP_NAME + " " + APP_EDITION_NAME + " " + APP_VERSION + " " + APP_OS_NAME + " " + APP_CPU_NAME;

	// Parse command line arguments
	static const struct option longOptions[] = {
		{"safe", no_argument, NULL, 'a'},
		{"dev", no_argument, NULL, 'd'},
		{"headless", no_argument, NULL, 'h'},
		{"screenshot", required_argument, NULL, 't'},
		{"system", required_argument, NULL, 's'},
		{"user", required_argument, NULL, 'u'},
		{"version", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, 256},
		{NULL, 0, NULL, 0}
	};
	int c;
	opterr = 0;

	while ((c = getopt_long(argc, argv, "adht:s:u:vp:", longOptions, NULL)) != -1) {
		switch (c) {
			case 'a': {
				settings::safeMode = true;
			} break;
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
			case 'v': {
				std::fprintf(stderr, "%s\n", appInfo.c_str());
				return 0;
			}
			case 256: { // --help
				std::fprintf(stderr, "%s\n", appInfo.c_str());
				std::fprintf(stderr, "https://vcvrack.com/manual/Installing#Command-line-usage\n");
				return 0;
			}
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
	if (!settings::devMode) {
		logger::logPath = asset::user("log.txt");
	}
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
	INFO("%s", appInfo.c_str());
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
	INFO("System time: %s", string::formatTimeISO(system::getUnixTime()).c_str());

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
	if (!system::isDirectory(resDir)) {
		std::string message = string::f("Rack's resource directory \"%s\" does not exist. Make sure Rack is correctly installed and launched.", resDir.c_str());
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, message.c_str());
		exit(1);
	}

	INFO("Initializing network");
	network::init();
	INFO("Initializing audio");
	audio::init();
	rtaudioInit();
	INFO("Initializing MIDI");
	midi::init();
	rtmidiInit();
	keyboard::init();
	gamepad::init();
	midiloopback::init();
	INFO("Initializing plugins");
	plugin::init();
	INFO("Initializing browser");
	app::browserInit();
	INFO("Initializing library");
	library::init();
	if (!settings::headless) {
		INFO("Initializing UI");
		ui::init();
		INFO("Initializing window");
		window::init();
	}

	// Initialize context
	contextSet(new Context);
	INFO("Creating MIDI loopback");
	APP->midiLoopbackContext = new midiloopback::Context;
	INFO("Creating engine");
	APP->engine = new engine::Engine;
	INFO("Creating history state");
	APP->history = new history::State;
	INFO("Creating event state");
	APP->event = new widget::EventState;
	INFO("Creating scene");
	APP->scene = new app::Scene;
	APP->event->rootWidget = APP->scene;
	INFO("Creating patch manager");
	APP->patch = new patch::Manager;
	if (!settings::headless) {
		INFO("Creating window");
		APP->window = new window::Window;
	}

	// On Mac, use a hacked-in GLFW addition to get the launched path.
#if defined ARCH_MAC
	// For some reason, launching from the command line sets glfwGetOpenedFilenames(), so make sure we're running the app bundle.
	if (asset::bundlePath != "") {
		const char* const* openedFilenames = glfwGetOpenedFilenames();
		if (openedFilenames && openedFilenames[0]) {
			patchPath = openedFilenames[0];
		}
	}
#endif

	// Initialize patch
	if (logger::wasTruncated() && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack crashed during the last session, possibly due to a buggy module in your patch. Clear your patch and start over?")) {
		// Do nothing, which leaves a blank patch
	}
	else {
		APP->patch->launch(patchPath);
	}

	APP->engine->startFallbackThread();

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
		// APP->window = new window::Window;
		// APP->window->run();
	}

	// Destroy context
	INFO("Deleting context");
	delete APP;
	contextSet(NULL);
	if (!settings::headless) {
		settings::save();
	}

	// Destroy environment
	if (!settings::headless) {
		INFO("Destroying window");
		window::destroy();
		INFO("Destroying UI");
		ui::destroy();
	}
	INFO("Destroying library");
	library::destroy();
	INFO("Destroying MIDI");
	midi::destroy();
	INFO("Destroying audio");
	audio::destroy();
	INFO("Destroying plugins");
	plugin::destroy();
	INFO("Destroying network");
	network::destroy();
	settings::destroy();
	INFO("Destroying logger");
	logger::destroy();

	return 0;
}


#ifdef UNICODE
/** UTF-16 to UTF-8 wrapper for Windows with unicode */
int wmain(int argc, wchar_t* argvU16[]) {
	// Initialize char* array with string-owned buffers
	std::string argvStr[argc];
	const char* argvU8[argc + 1];
	for (int i = 0; i < argc; i++) {
		argvStr[i] = string::UTF16toUTF8(argvU16[i]);
		argvU8[i] = argvStr[i].c_str();
	}
	argvU8[argc] = NULL;
	return main(argc, (char**) argvU8);
}
#endif
