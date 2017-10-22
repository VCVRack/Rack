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
/** Searches for a manufacturer resource, given a Manufacturer object
*/
std::string assetManufacturer(Manufacturer *manufacturer, std::string filename);


} // namespace rack
