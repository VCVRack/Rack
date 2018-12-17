#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"


namespace rack {
namespace asset {


void init(bool devMode);
/** Returns the path of a global resource. Should only read files from this location. */
std::string global(std::string filename);
/** Returns the path of a local resource. Can read and write files to this location. */
std::string local(std::string filename);
/** Returns the path of a resource in the plugin's folder. Should only read files from this location. */
std::string plugin(Plugin *plugin, std::string filename);


extern std::string globalDir;
extern std::string localDir;


} // namespace asset
} // namespace rack
