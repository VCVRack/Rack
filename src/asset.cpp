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

#include <asset.hpp>
#include <system.hpp>
#include <settings.hpp>
#include <string.hpp>
#include <plugin/Plugin.hpp>
#include <engine/Module.hpp>
#include <app/common.hpp>


namespace rack {
namespace asset {


static void initSystemDir() {
	if (!systemDir.empty())
		return;

	if (settings::devMode) {
		systemDir = system::getWorkingDirectory();
		return;
	}

	// Environment variable overrides
	const char* env = getenv("RACK_SYSTEM_DIR");
	if (env) {
		systemDir = env;
		return;
	}

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
	wchar_t moduleBufW[MAX_PATH] = L"";
	DWORD length = GetModuleFileNameW(NULL, moduleBufW, LENGTHOF(moduleBufW));
	assert(length > 0);
	// Get directory of executable
	PathRemoveFileSpecW(moduleBufW);
	systemDir = string::UTF16toUTF8(moduleBufW);
#endif
#if defined ARCH_LIN
	// Use the current working directory as the default path on Linux.
	systemDir = system::getWorkingDirectory();
#endif
}


static void initUserDir() {
	if (!userDir.empty())
		return;

	if (settings::devMode) {
		userDir = systemDir;
		return;
	}

	// Environment variable overrides
	const char* env = getenv("RACK_USER_DIR");
	if (env) {
		userDir = env;
		return;
	}

#if defined ARCH_WIN
	// Get "My Documents" path
	wchar_t documentsBufW[MAX_PATH] = L".";
	HRESULT result = SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documentsBufW);
	assert(result == S_OK);
	userDir = system::join(string::UTF16toUTF8(documentsBufW), "Rack" + APP_VERSION_MAJOR);
#endif
#if defined ARCH_MAC
	// Get home directory
	struct passwd* pw = getpwuid(getuid());
	assert(pw);
	userDir = system::join(pw->pw_dir, "Documents", "Rack" + APP_VERSION_MAJOR);
#endif
#if defined ARCH_LIN
	// Get home directory
	const char* homeBuf = getenv("HOME");
	if (!homeBuf) {
		struct passwd* pw = getpwuid(getuid());
		assert(pw);
		homeBuf = pw->pw_dir;
	}
	userDir = system::join(homeBuf, ".Rack" + APP_VERSION_MAJOR);
#endif
}

void init() {
	initSystemDir();
	initUserDir();

	system::createDirectory(userDir);
}


std::string system(std::string filename) {
	return system::join(systemDir, filename);
}


std::string user(std::string filename) {
	return system::join(userDir, filename);
}


std::string plugin(plugin::Plugin* plugin, std::string filename) {
	assert(plugin);
	return system::join(plugin->path, filename);
}


std::string systemDir;
std::string userDir;

std::string bundlePath;


} // namespace asset
} // namespace rack
