#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "settings.hpp"


#if ARCH_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h> // for chdir and access
#include <libgen.h> // for dirname
// #include <string.h>
#include <mach-o/dyld.h> // for _NSGetExecutablePath
#include <limits.h> // for PATH_MAX?
#include <dirent.h> // for opendir

void alert(std::string header, std::string message, int level) {
	CFStringRef headerRef = CFStringCreateWithCString(NULL, header.c_str(), header.size());
	CFStringRef messageRef = CFStringCreateWithCString(NULL, message.c_str(), message.size());
	CFOptionFlags result;

	CFUserNotificationDisplayAlert(
		0, // no timeout
		level, // flags for alert level
		NULL, // iconURL
		NULL, // soundURL
		NULL, // localizationURL
		headerRef,
		messageRef,
		NULL, // default "OK"
		NULL, // alternative button
		NULL, // other button
		&result
		);

	CFRelease(headerRef);
	CFRelease(messageRef);
}

bool isCorrectCwd() {
	DIR *dir = opendir("res");
	if (dir) {
		closedir(dir);
		return true;
	}
	else {
		return false;
	}
}

/** macOS workaround for setting the working directory to the location of the .app */
void fixCwd() {
	// Check if the cwd is already set correctly (e.g. launched from the command line or gdb)
	if (isCorrectCwd())
		return;

/*
	// Get path of binary inside the app bundle
	// It should be something like .../Rack.app/Contents/MacOS
	char path[PATH_MAX];
	uint32_t pathLen = sizeof(path);
	int err = _NSGetExecutablePath(path, &pathLen);
	assert(!err);
	if (isCorrectCwd())
		return;

	// Switch to the directory of the actual binary
	chdir(dirname(path));
	if (isCorrectCwd())
		return;

	// and then go up three directories to get to the parent directory
	chdir("../../../");
	if (isCorrectCwd())
		return;
*/

	// Switch to a default absolute path
	chdir("/Applications/Rack");
	if (isCorrectCwd())
		return;

	alert("Install Rack", "To install Rack, please move the Rack directory (including the Rack app and plugins directory) to the /Applications folder.", 2);
	exit(1);
}
#endif


#ifdef ARCH_LIN
// Tell Linux linker to request older version of glibc
__asm__(".symver realpath,realpath@GLIBC_2.2.5");
#endif


using namespace rack;

int main() {
#if ARCH_MAC
	fixCwd();
#endif

	pluginInit();
	engineInit();
	guiInit();
	sceneInit();
	settingsLoad("settings.json");
	gRackWidget->loadPatch("autosave.json");

	engineStart();
	guiRun();
	engineStop();

	gRackWidget->savePatch("autosave.json");
	settingsSave("settings.json");
	sceneDestroy();
	guiDestroy();
	engineDestroy();
	pluginDestroy();
	return 0;
}

