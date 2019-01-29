#pragma once
#include "plugin/Plugin.hpp"


/** Called once to initialize and return the Plugin instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::plugin::Plugin *plugin);
