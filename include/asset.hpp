#pragma once
#include <common.hpp>


namespace rack {


namespace plugin {
struct Plugin;
} // namespace plugin


namespace engine {
struct Module;
} // namespace engine


/** Handles common path locations */
namespace asset {


PRIVATE void init();

/** Returns the path of a system asset. Read-only files. */
std::string system(std::string filename = "");

/** Returns the path of a user asset. Readable/writable files. */
std::string user(std::string filename = "");

/** Returns the path of an asset in the plugin's dir. Read-only files.

Examples:

	asset::plugin(pluginInstance, "samples/00.wav") // "/<Rack user dir>/plugins/MyPlugin/samples/00.wav"
*/
std::string plugin(plugin::Plugin* plugin, std::string filename = "");


// Set these before calling init() to override the default paths
extern std::string systemDir;
extern std::string userDir;
// Only defined on Mac
extern std::string bundlePath;


} // namespace asset
} // namespace rack
