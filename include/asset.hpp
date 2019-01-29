#pragma once
#include "common.hpp"


namespace rack {


namespace plugin {
	struct Plugin;
} // namespace plugin


namespace asset {


void init(bool devMode);
/** Returns the path of a system resource. Should only read files from this location. */
std::string system(std::string filename);
/** Returns the path of a user resource. Can read and write files to this location. */
std::string user(std::string filename);
/** Returns the path of a resource in the plugin's folder. Should only read files from this location. */
std::string plugin(plugin::Plugin *plugin, std::string filename);


extern std::string systemDir;
extern std::string userDir;


} // namespace asset
} // namespace rack
