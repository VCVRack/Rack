#if defined(APPLE)
	#include "CoreFoundation/CoreFoundation.h"
	#include <unistd.h>
	#include <libgen.h>
#endif

#include "Rack.hpp"


using namespace rack;

int main() {
	// Set working directory
#if defined(APPLE)
	{
		CFBundleRef bundle = CFBundleGetMainBundle();
		CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
		char path[PATH_MAX];
		Boolean success = CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX);
		assert(success);
		CFRelease(bundleURL);

		// chdir(dirname(path));
	}
#endif


	pluginInit();
	rackInit();
	guiInit();
	gRackWidget->loadPatch("autosave.json");

	rackStart();
	guiRun();
	rackStop();

	gRackWidget->savePatch("autosave.json");
	guiDestroy();
	rackDestroy();
	pluginDestroy();
	return 0;
}
