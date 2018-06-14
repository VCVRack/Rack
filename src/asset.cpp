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


void assetInit(bool devMode, std::string customGlobalDir, std::string customLocalDir) {
	if (devMode) {
		// Use current working directory if running in development mode
		globalDir = ".";
		localDir = ".";
		return;
	}

	if (customGlobalDir.empty()) {

#if ARCH_MAC
	  CFBundleRef bundle = CFBundleGetMainBundle();
	  assert(bundle);
	  CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
	  char resourcesBuf[PATH_MAX];
	  Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8*) resourcesBuf, sizeof(resourcesBuf));
	  assert(success);
	  CFRelease(resourcesUrl);
	  globalDir = resourcesBuf;
#endif
#if ARCH_WIN
	  char moduleBuf[MAX_PATH];
	  DWORD length = GetModuleFileName(NULL, moduleBuf, sizeof(moduleBuf));
	  assert(length > 0);
	  PathRemoveFileSpec(moduleBuf);
	  globalDir = moduleBuf;
#endif
#if ARCH_LIN
	  // TODO For now, users should launch Rack from their terminal in the global directory
	  globalDir = ".";
#endif
	}
	else {
	  globalDir = customGlobalDir;
	}

	if (customGlobalDir.empty()) {
#if ARCH_MAC
	  // Get home directory
	  struct passwd *pw = getpwuid(getuid());
	  assert(pw);
	  localDir = pw->pw_dir;
	  localDir += "/Documents/Rack";
#endif
#if ARCH_WIN
	  // Get "My Documents" folder
	  char documentsBuf[MAX_PATH];
	  HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBuf);
	  assert(result == S_OK);
	  localDir = documentsBuf;
	  localDir += "/Rack";
#endif
#if ARCH_LIN
	  // Get home directory
	  const char *homeBuf = getenv("HOME");
	  if (!homeBuf) {
	    struct passwd *pw = getpwuid(getuid());
	    assert(pw);
	    homeBuf = pw->pw_dir;
	  }
	  localDir = homeBuf;
	  localDir += "/.Rack";
#endif
	}
	else {
	  localDir = customLocalDir;
	}
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
