#pragma once
#include <common.hpp>


namespace rack {


namespace plugin {
struct Plugin;
} // namespace plugin


namespace engine {
struct Module;
} // namespace engine


namespace asset {


void init();

/** Returns the path of a system asset. Should only read files from this location. */
std::string system(std::string filename = "");

/** Returns the path of a user asset. Can read and write files to this location. */
std::string user(std::string filename = "");

/** Returns the path of a asset in the plugin's folder.
Plugin assets should be read-only by plugins.
Examples:
	asset::plugin(pluginInstance, "samples/00.wav") // "/path/to/Rack/user/folder/plugins/MyPlugin/samples/00.wav"
*/
std::string plugin(plugin::Plugin* plugin, std::string filename = "");

/** Returns the path to an asset in the module patch folder.
The module patch folder is *not* created automatically. Before creating files at these paths, call
	system::createDirectories(asset::module(module))

Examples:
	asset::module(module, "recordings/00.wav") // "/path/to/Rack/user/folder/autosave/modules/1234/recordings/00.wav"
*/
std::string module(engine::Module* module, const std::string& filename = "");


// Set these before calling init() to override the default paths
extern std::string systemDir;
extern std::string userDir;

extern std::string logPath;
extern std::string pluginsPath;
extern std::string settingsPath;
extern std::string autosavePath;
extern std::string templatePath;
// Only defined on Mac
extern std::string bundlePath;


} // namespace asset
} // namespace rack
