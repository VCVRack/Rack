#pragma once
#include <plugin/Plugin.hpp>


/** Called immediately after loading your plugin.

Use this to save `plugin` to a global variable and add Models to it.
Required in plugins.
*/
extern "C"
void init(rack::plugin::Plugin* plugin);

/** Called before your plugin library is unloaded.

Optional in plugins.
*/
extern "C"
void destroy();
