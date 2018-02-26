#include "asset.hpp"
#include "util/common.hpp"
#include <sys/stat.h> // for mkdir
#include "osdialog.h"

#if ARCH_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <pwd.h>
#endif

#if ARCH_WIN
#include <Windows.h>
#include <Shlobj.h>
#endif

#if ARCH_LIN
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif


namespace rack {


std::string assetGlobal(std::string filename) {
	std::string dir;
#if defined(RELEASE)
#if ARCH_MAC
	CFBundleRef bundle = CFBundleGetMainBundle();
	assert(bundle);
	CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
	char buf[PATH_MAX];
	Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8 *)buf, sizeof(buf));
	assert(success);
	CFRelease(resourcesUrl);
	dir = buf;
#endif
#if ARCH_WIN
	// Must launch Rack with the "Start In" directory as the global directory
	dir = ".";
#endif
#if ARCH_LIN
	// TODO For now, users should launch Rack from their terminal in the global directory
	dir = ".";
#endif
#else // RELEASE
	dir = ".";
#endif // RELEASE
	return dir + "/" + filename;
}


std::string assetLocal(std::string filename) {
	std::string dir;
#if defined(RELEASE)
#if ARCH_MAC
	// Get home directory
	struct passwd *pw = getpwuid(getuid());
	assert(pw);
	dir = pw->pw_dir;
	dir += "/Documents/Rack";
	mkdir(dir.c_str(), 0755);
#endif
#if ARCH_WIN
	// Get "My Documents" folder
	char buf[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, buf);
	assert(result == S_OK);
	dir = buf;
	dir += "/Rack";
	CreateDirectory(dir.c_str(), NULL);
#endif
#if ARCH_LIN
	const char *home = getenv("HOME");
	if (!home) {
		struct passwd *pw = getpwuid(getuid());
		assert(pw);
		home = pw->pw_dir;
	}
	dir = home;
	dir += "/.Rack";
	mkdir(dir.c_str(), 0755);
#endif
#else // RELEASE
	dir = ".";
#endif // RELEASE
	return dir + "/" + filename;
}


std::string assetPlugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


} // namespace rack
