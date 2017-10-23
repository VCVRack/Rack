#pragma once

#include <string>
#include "plugin.hpp"


namespace rack {


/** Returns the path of a global resource. Read-only
*/
std::string assetGlobal(std::string filename);
/** Returns the path of a local resource. Read/write
*/
std::string assetLocal(std::string filename);
/** Returns the path of a resource in the plugin's folder. Read-only
*/
std::string assetPlugin(Plugin *plugin, std::string filename);


} // namespace rack
