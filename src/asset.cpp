#include "global_pre.hpp"
#include "asset.hpp"
#include "util/common.hpp"
#include "osdialog.h"
#include "global.hpp"

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


void assetInit(bool devMode) {
	if (devMode) {
		// Use current working directory if running in development mode
		global->asset.globalDir = ".";
		global->asset.localDir = ".";
		return;
	}

#if ARCH_MAC
	CFBundleRef bundle = CFBundleGetMainBundle();
	assert(bundle);
	CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
	char resourcesBuf[PATH_MAX];
	Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8*) resourcesBuf, sizeof(resourcesBuf));
	assert(success);
	CFRelease(resourcesUrl);
	global->asset.globalDir = resourcesBuf;

	// Get home directory
	struct passwd *pw = getpwuid(getuid());
	assert(pw);
	global->asset.localDir = pw->pw_dir;
	global->asset.localDir += "/Documents/Rack";
#endif
#if ARCH_WIN
#ifndef USE_VST2
	char moduleBuf[MAX_PATH];
	DWORD length = GetModuleFileName(NULL, moduleBuf, sizeof(moduleBuf));
	assert(length > 0);
	PathRemoveFileSpec(moduleBuf);
	global->asset.globalDir = moduleBuf;
	// Get "My Documents" folder
	char documentsBuf[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBuf);
	assert(result == S_OK);
	global->asset.localDir = documentsBuf;
	global->asset.localDir += "/Rack";
#else
	global->asset.globalDir = global->vst2.program_dir; 
   global->asset.localDir = global->vst2.program_dir;
#endif // USE_VST2

#endif
#if ARCH_LIN
	// TODO For now, users should launch Rack from their terminal in the global directory
	global->asset.globalDir = ".";

	// Get home directory
	const char *homeBuf = getenv("HOME");
	if (!homeBuf) {
		struct passwd *pw = getpwuid(getuid());
		assert(pw);
		homeBuf = pw->pw_dir;
	}
	global->asset.localDir = homeBuf;
	global->asset.localDir += "/.Rack";
#endif
	systemCreateDirectory(global->asset.localDir);
}


std::string assetGlobal(std::string filename) {
	return global->asset.globalDir + "/" + filename;
}


std::string assetLocal(std::string filename) {
	return global->asset.localDir + "/" + filename;
}


std::string assetPlugin(Plugin *plugin, std::string filename) {
   printf("xxx assetPlugin(plugin=%p)\n");
   printf("xxx assetPlugin: filename=\"%s\"\n", filename.c_str());
	assert(plugin);
	return plugin->path + "/" + filename;
}

#ifdef USE_VST2
std::string assetStaticPlugin(const char *name/*e.g. "Fundamentals"*/, const char *_relPathOrNull) {
	return global->asset.localDir + "plugins/" + name + "/" + ((NULL != _relPathOrNull)?_relPathOrNull:"");
}
std::string assetPlugin(const char *name/*e.g. "Fundamentals"*/, const char *_relPathOrNull) {
	return assetStaticPlugin(name, _relPathOrNull);
}
#endif // USE_VST2


} // namespace rack
