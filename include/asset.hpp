#pragma once

#include <string>
#include "plugin.hpp"


namespace rack {


void assetInit(bool devMode);
/** Returns the path of a global resource. Should only read files from this location. */
std::string assetGlobal(std::string filename);
/** Returns the path of a local resource. Can read and write files to this location. */
std::string assetLocal(std::string filename);
/** Returns the path of a resource in the plugin's folder. Should only read files from this location. */
std::string assetPlugin(Plugin *plugin, std::string filename);

#ifdef USE_VST2
std::string assetStaticPlugin(const char *name/*e.g. "Fundamentals"*/, const char *_relPathOrNull = 0);
std::string assetPlugin(const char *name/*e.g. "Fundamentals"*/, const char *_relPathOrNull = 0);
#endif // USE_VST2


} // namespace rack
