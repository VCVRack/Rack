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

/** Called when saving user settings.
Stored in `settings["pluginSettings"][pluginSlug]`.
Useful for persisting plugin-wide settings.

Optional in plugins.
*/
extern "C"
json_t* settingsToJson();

/** Called after initializing plugin if user plugin settings property is defined.

Optional in plugins.
*/
extern "C"
void settingsFromJson(json_t* rootJ);
