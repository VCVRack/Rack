#include "asset.hpp"
#include "system.hpp"
#include "plugin/Plugin.hpp"

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
namespace asset {


void init(bool devMode) {
	// Get system dir
	if (gSystemDir.empty()) {
		if (devMode) {
			gSystemDir = ".";
		}
		else {
#if ARCH_MAC
			CFBundleRef bundle = CFBundleGetMainBundle();
			assert(bundle);
			CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
			char resourcesBuf[PATH_MAX];
			Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8*) resourcesBuf, sizeof(resourcesBuf));
			assert(success);
			CFRelease(resourcesUrl);
			gSystemDir = resourcesBuf;
#endif
#if ARCH_WIN
			char moduleBuf[MAX_PATH];
			DWORD length = GetModuleFileName(NULL, moduleBuf, sizeof(moduleBuf));
			assert(length > 0);
			PathRemoveFileSpec(moduleBuf);
			gSystemDir = moduleBuf;
#endif
#if ARCH_LIN
			// TODO For now, users should launch Rack from their terminal in the system directory
			gSystemDir = ".";
#endif
		}
	}

	// Get user dir
	if (gUserDir.empty()) {
		if (devMode) {
			gUserDir = ".";
		}
		else {
#if ARCH_WIN
			// Get "My Documents" folder
			char documentsBuf[MAX_PATH];
			HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBuf);
			assert(result == S_OK);
			gUserDir = documentsBuf;
			gUserDir += "/Rack";
#endif
#if ARCH_MAC
			// Get home directory
			struct passwd *pw = getpwuid(getuid());
			assert(pw);
			gUserDir = pw->pw_dir;
			gUserDir += "/Documents/Rack";
#endif
#if ARCH_LIN
			// Get home directory
			const char *homeBuf = getenv("HOME");
			if (!homeBuf) {
				struct passwd *pw = getpwuid(getuid());
				assert(pw);
				homeBuf = pw->pw_dir;
			}
			gUserDir = homeBuf;
			gUserDir += "/.Rack";
#endif
		}
	}

	system::createDirectory(gSystemDir);
	system::createDirectory(gUserDir);
}


std::string system(std::string filename) {
	return gSystemDir + "/" + filename;
}


std::string user(std::string filename) {
	return gUserDir + "/" + filename;
}


std::string plugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


std::string gSystemDir;
std::string gUserDir;


} // namespace asset
} // namespace rack
