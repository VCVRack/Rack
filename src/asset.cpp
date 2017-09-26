#include "asset.hpp"
#include <stdlib.h> // for realpath
#include <assert.h>


namespace rack {


static std::string fileRealPath(std::string path) {
	char *rpath = realpath(path.c_str(), NULL);
	if (!rpath)
		return "";

	std::string rrpath = path;
	free(rpath);
	return rrpath;
}

std::string assetGlobal(std::string filename) {
	std::string path;
#if ARCH_MAC
	// TODO
#endif
#if ARCH_WIN
	path = "./" + filename;
#endif
#if ARCH_LIN
	path = "./" + filename;
#endif
	return fileRealPath(path);
}

std::string assetLocal(std::string filename) {
	std::string path;
#if ARCH_MAC
	// TODO
#endif
#if ARCH_WIN
	// TODO
#endif
#if ARCH_LIN
	// TODO
	// If Rack is "installed" (however that may be defined), look in ~/.Rack or something instead
	path = "./" + filename;
#endif
	return fileRealPath(path);
}

std::string assetPlugin(Plugin *plugin, std::string filename) {
	assert(plugin);
	return fileRealPath(plugin->path + "/" + filename);
}


} // namespace rack
