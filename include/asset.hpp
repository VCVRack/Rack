#pragma once

#include <string>
#include "plugin.hpp"


namespace rack {


/** Searches for a global read-only resource and returns its path, or "" if not found
*/
std::string assetGlobal(std::string filename);
/** Searches for a local resource
*/
std::string assetLocal(std::string filename);
/** Searches for a plugin resource, given a Plugin object
*/
std::string assetPlugin(Plugin *plugin, std::string filename);


} // namespace rack
