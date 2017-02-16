#if defined(APPLE)
	#include "CoreFoundation/CoreFoundation.h"
	#include <unistd.h>
	#include <libgen.h>
#endif

#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "plugin.hpp"


using namespace rack;

int main() {
#if defined(APPLE)
	// macOS workaround for setting the working directory to the location of the .app
	{
		CFBundleRef bundle = CFBundleGetMainBundle();
		CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
		char path[PATH_MAX];
		Boolean success = CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX);
		assert(success);
		CFRelease(bundleURL);

		chdir(dirname(path));
	}
#endif

	engineInit();
	guiInit();
	sceneInit();
	pluginInit();
	gRackWidget->loadPatch("autosave.json");

	engineStart();
	guiRun();
	engineStop();

	gRackWidget->savePatch("autosave.json");
	sceneDestroy();
	guiDestroy();
	engineDestroy();
	pluginDestroy();
	return 0;
}

