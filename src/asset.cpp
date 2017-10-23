#include "asset.hpp"
#include "util.hpp"
#include <assert.h>
#include <sys/stat.h> // for mkdir
#include "../ext/osdialog/osdialog.h"

#if ARCH_MAC
	#include <CoreFoundation/CoreFoundation.h>
	#include <pwd.h>
#endif


namespace rack {


#if ARCH_MAC
/** Is it actually difficult to determine whether we are running in a Mac bundle or not.
This heuristically guesses based on the existence of a Resources directory
*/
static bool isBundle() {
	CFBundleRef bundle = CFBundleGetMainBundle();
	if (bundle) {
		CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
		char buf[PATH_MAX];
		Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8 *)buf, sizeof(buf));
		assert(success);
		CFRelease(resourcesUrl);
		if (extractFilename(buf) == "Resources")
			return true;
	}
	return false;
}
#endif


std::string assetGlobal(std::string filename) {
	std::string path;
#if ARCH_MAC
	CFBundleRef bundle = CFBundleGetMainBundle();
	if (bundle && isBundle()) {
		CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
		char buf[PATH_MAX];
		Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8 *)buf, sizeof(buf));
		assert(success);
		CFRelease(resourcesUrl);
		path = buf;
	}
	else {
		path = ".";
	}
	path += "/" + filename;
#endif
#if ARCH_WIN
	path = "./" + filename;
#endif
#if ARCH_LIN
	path = "./" + filename;
#endif
	return path;
}

std::string assetLocal(std::string filename) {
	std::string path;
#if ARCH_MAC
	if (isBundle()) {
		// Get home directory
		struct passwd *pw = getpwuid(getuid());
		assert(pw);
		path = pw->pw_dir;
		path += "/Documents/Rack";
		mkdir(path.c_str(), 0755);
	}
	else {
		path = ".";
	}
	path += "/" + filename;
#endif
#if ARCH_WIN
	// TODO
	// Use ~/My Documents/Rack or something
	path = "./" + filename;
#endif
#if ARCH_LIN
	// TODO
	// If Rack is "installed" (however that may be defined), look in ~/.Rack or something instead
	path = "./" + filename;
#endif
	return path;
}

std::string assetPlugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	std::string path;
	path = plugin->path + "/" + filename;
	return path;
}


} // namespace rack
