#include <asset.hpp>
#include <system.hpp>
#include <settings.hpp>
#include <string.hpp>
#include <plugin/Plugin.hpp>
#include <app/common.hpp>

#if defined ARCH_MAC
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreServices/CoreServices.h>
	#include <pwd.h>
#endif

#if defined ARCH_WIN
	#include <Windows.h>
	#include <Shlobj.h>
	#include <Shlwapi.h>
#endif

#if defined ARCH_LIN
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
#endif


namespace rack {
namespace asset {


void init() {
	// Get system dir
	if (systemDir.empty()) {
		if (settings::devMode) {
			systemDir = ".";
		}
		else {
#if defined ARCH_MAC
			CFBundleRef bundle = CFBundleGetMainBundle();
			assert(bundle);

			// Check if we're running as a command-line program or an app bundle.
			CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
			// Thanks Ken Thomases! https://stackoverflow.com/a/58369256/272642
			CFStringRef uti;
			if (CFURLCopyResourcePropertyForKey(bundleUrl, kCFURLTypeIdentifierKey, &uti, NULL) && uti && UTTypeConformsTo(uti, kUTTypeApplicationBundle)) {
				char bundleBuf[PATH_MAX];
				Boolean success = CFURLGetFileSystemRepresentation(bundleUrl, TRUE, (UInt8*) bundleBuf, sizeof(bundleBuf));
				assert(success);
				bundlePath = bundleBuf;
			}

			CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
			char resourcesBuf[PATH_MAX];
			Boolean success = CFURLGetFileSystemRepresentation(resourcesUrl, TRUE, (UInt8*) resourcesBuf, sizeof(resourcesBuf));
			assert(success);
			CFRelease(resourcesUrl);
			systemDir = resourcesBuf;
#endif
#if defined ARCH_WIN
			// Get path to executable
			wchar_t moduleBufW[MAX_PATH];
			DWORD length = GetModuleFileNameW(NULL, moduleBufW, LENGTHOF(moduleBufW));
			assert(length > 0);
			// Get folder of executable
			PathRemoveFileSpecW(moduleBufW);
			// Convert to short path to avoid Unicode
			wchar_t moduleBufShortW[MAX_PATH];
			GetShortPathNameW(moduleBufW, moduleBufShortW, LENGTHOF(moduleBufShortW));
			systemDir = string::fromWstring(moduleBufShortW);
#endif
#if defined ARCH_LIN
			// Users should launch Rack from their terminal in the system directory
			systemDir = ".";
#endif
		}
	}

	// Get user dir
	if (userDir.empty()) {
		if (settings::devMode) {
			userDir = ".";
		}
		else {
#if defined ARCH_WIN
			// Get "My Documents" folder
			wchar_t documentsBufW[MAX_PATH] = L".";
			HRESULT result = SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBufW);
			assert(result == S_OK);
			// Convert to short path to avoid Unicode
			wchar_t documentsBufShortW[MAX_PATH];
			GetShortPathNameW(documentsBufW, documentsBufShortW, LENGTHOF(documentsBufShortW));
			userDir = string::fromWstring(documentsBufShortW);
			userDir += "/Rack";
#endif
#if defined ARCH_MAC
			// Get home directory
			struct passwd* pw = getpwuid(getuid());
			assert(pw);
			userDir = pw->pw_dir;
			userDir += "/Documents/Rack";
#endif
#if defined ARCH_LIN
			// Get home directory
			const char* homeBuf = getenv("HOME");
			if (!homeBuf) {
				struct passwd* pw = getpwuid(getuid());
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

	// Set paths
	if (settings::devMode) {
		pluginsPath = userDir + "/plugins";
		settingsPath = userDir + "/settings.json";
		autosavePath = userDir + "/autosave.vcv";
		templatePath = userDir + "/template.vcv";
	}
	else {
		logPath = userDir + "/log.txt";
		pluginsPath = userDir + "/plugins-v" + app::ABI_VERSION;
		settingsPath = userDir + "/settings-v" + app::ABI_VERSION + ".json";
		autosavePath = userDir + "/autosave-v" + app::ABI_VERSION + ".vcv";
		templatePath = userDir + "/template-v" + app::ABI_VERSION + ".vcv";
	}
}


std::string system(std::string filename) {
	return systemDir + "/" + filename;
}


std::string user(std::string filename) {
	return userDir + "/" + filename;
}


std::string plugin(plugin::Plugin* plugin, std::string filename) {
	assert(plugin);
	return plugin->path + "/" + filename;
}


std::string systemDir;
std::string userDir;

std::string logPath;
std::string pluginsPath;
std::string settingsPath;
std::string autosavePath;
std::string templatePath;
std::string bundlePath;


} // namespace asset
} // namespace rack
