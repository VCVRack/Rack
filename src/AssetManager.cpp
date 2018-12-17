#include "AssetManager.hpp"
#include "system.hpp"
#include "context.hpp"
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


AssetManager::AssetManager() {
	// Get system dir
	if (systemDir.empty()) {
		if (context()->devMode) {
			systemDir = ".";
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
			systemDir = resourcesBuf;
#endif
#if ARCH_WIN
			char moduleBuf[MAX_PATH];
			DWORD length = GetModuleFileName(NULL, moduleBuf, sizeof(moduleBuf));
			assert(length > 0);
			PathRemoveFileSpec(moduleBuf);
			systemDir = moduleBuf;
#endif
#if ARCH_LIN
			// TODO For now, users should launch Rack from their terminal in the system directory
			systemDir = ".";
#endif
		}
	}

	// Get user dir
	if (userDir.empty()) {
		if (context()->devMode) {
			userDir = ".";
		}
		else {
#if ARCH_WIN
			// Get "My Documents" folder
			char documentsBuf[MAX_PATH];
			HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBuf);
			assert(result == S_OK);
			userDir = documentsBuf;
			userDir += "/Rack";
#endif
#if ARCH_MAC
			// Get home directory
			struct passwd *pw = getpwuid(getuid());
			assert(pw);
			userDir = pw->pw_dir;
			userDir += "/Documents/Rack";
#endif
#if ARCH_LIN
			// Get home directory
			const char *homeBuf = getenv("HOME");
			if (!homeBuf) {
				struct passwd *pw = getpwuid(getuid());
				assert(pw);
				homeBuf = pw->pw_dir;
			}
			userDir = homeBuf;
			userDir += "/.Rack";
#endif
		}
	}

	system::createDirectory(systemDir);
	system::createDirectory(userDir);
}


std::string AssetManager::system(std::string filename) {
	return systemDir + "/" + filename;
}


std::string AssetManager::user(std::string filename) {
	return userDir + "/" + filename;
}


std::string AssetManager::plugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


} // namespace rack
