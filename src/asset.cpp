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


std::string assetGlobalDir;
std::string assetLocalDir;


void assetInit(bool devMode) {
	if (assetGlobalDir.empty()) {
		if (devMode) {
			assetGlobalDir = ".";
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
			assetGlobalDir = resourcesBuf;
#endif
#if ARCH_WIN
			char moduleBuf[MAX_PATH];
			DWORD length = GetModuleFileName(NULL, moduleBuf, sizeof(moduleBuf));
			assert(length > 0);
			PathRemoveFileSpec(moduleBuf);
			assetGlobalDir = moduleBuf;
#endif
#if ARCH_LIN
			// TODO For now, users should launch Rack from their terminal in the global directory
			assetGlobalDir = ".";
#endif
		}
	}

	if (assetLocalDir.empty()) {
		if (devMode) {
			assetLocalDir = ".";
		}
		else {
#if ARCH_WIN
			// Get "My Documents" folder
			char documentsBuf[MAX_PATH];
			HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBuf);
			assert(result == S_OK);
			assetLocalDir = documentsBuf;
			assetLocalDir += "/Rack";
#endif
#if ARCH_MAC
			// Get home directory
			struct passwd *pw = getpwuid(getuid());
			assert(pw);
			assetLocalDir = pw->pw_dir;
			assetLocalDir += "/Documents/Rack";
#endif
#if ARCH_LIN
			// Get home directory
			const char *homeBuf = getenv("HOME");
			if (!homeBuf) {
				struct passwd *pw = getpwuid(getuid());
				assert(pw);
				homeBuf = pw->pw_dir;
			}
			assetLocalDir = homeBuf;
			assetLocalDir += "/.Rack";
#endif
		}
	}

	systemCreateDirectory(assetGlobalDir);
	systemCreateDirectory(assetLocalDir);
}


std::string assetGlobal(std::string filename) {
	return assetGlobalDir + "/" + filename;
}


std::string assetLocal(std::string filename) {
	return assetLocalDir + "/" + filename;
}


std::string assetPlugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


} // namespace rack
