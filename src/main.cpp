#if defined(APPLE)
	#include "CoreFoundation/CoreFoundation.h"
	#include <unistd.h>
	#include <libgen.h>
#endif

#include "rack.hpp"


namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion = "v0.1.0 alpha";

Rack *gRack;

} // namespace rack


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

	gRack = new Rack();
	guiInit();
	pluginInit();
	gRackWidget->loadPatch("autosave.json");

	gRack->start();
	guiRun();
	gRack->stop();

	gRackWidget->savePatch("autosave.json");
	guiDestroy();
	delete gRack;
	pluginDestroy();
	return 0;
}

