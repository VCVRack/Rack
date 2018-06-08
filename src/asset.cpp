#include "asset.hpp"
#include "util/common.hpp"
#include "osdialog.h"

#if ARCH_MAC
	#include <CoreFoundation/CoreFoundation.h>
	#include <pwd.h>
#endif

#if ARCH_WIN
	#include <Windows.h>
	#include <Shlobj.h>
	#include <Shlwapi.h>
#endif

#if ARCH_LIN
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
#endif


namespace rack {


static std::string globalDir;
static std::string localDir;


void assetInit(bool devMode) {
	if (devMode) {
		// Use current working directory if running in development mode
		globalDir = ".";
		localDir = ".";
		return;
	}

#if ARCH_MAC
	CFBundleRef bundle = CFBundleGetMainBundle();
	assert(bundle);
	CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
	char buf[PATH_MAX];
	Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8*) buf, sizeof(buf));
	assert(success);
	CFRelease(resourcesUrl);
	globalDir = buf;

	// Get home directory
	struct passwd *pw = getpwuid(getuid());
	assert(pw);
	localDir = pw->pw_dir;
	localDir += "/Documents/Rack";
#endif
#if ARCH_WIN
	char buf[MAX_PATH];
	DWORD length = GetModuleFileName(NULL, buf, sizeof(buf));
	assert(length > 0);
	PathRemoveFileSpec(buf);
	globalDir = buf;

	// Get "My Documents" folder
	char buf[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, buf);
	assert(result == S_OK);
	localDir = buf;
	localDir += "/Rack";
#endif
#if ARCH_LIN
	// TODO For now, users should launch Rack from their terminal in the global directory
	globalDir = ".";

	// Get home directory
	const char *home = getenv("HOME");
	if (!home) {
		struct passwd *pw = getpwuid(getuid());
		assert(pw);
		home = pw->pw_dir;
	}
	localDir = home;
	localDir += "/.Rack";
#endif
}


std::string assetGlobal(std::string filename) {
	return globalDir + "/" + filename;
}


std::string assetLocal(std::string filename) {
	return localDir + "/" + filename;
}


std::string assetPlugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


} // namespace rack
