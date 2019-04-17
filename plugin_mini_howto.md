
# Plugin Mini-HowTo


## How to add new modules to VeeSeeVSTRack via dynamically loaded plugin DLLs

( $(PLUGINNAME) refers to the plugin's name, e.g. "my_unique_plugin_name". $(MODULENAME) refers to the respective module name, e.g. "my_lfo" )

1. Either clone the GitHub repo OR download the [precompiled plugin SDK](README.md#dynamically-loaded-plugins-via-plugin-sdk)

2. Study the [template_shared](plugins/community/repos/Template_shared) example plugin (also included in the plugin SDK) (you may also want to take a look at my `bsp` plugin, which is also loaded dynamically). Also take a look at [include/plugin.hpp](include/plugin.hpp).

3. Develop your module (!). Don't forget to pick a suitable license, e.g. [MIT](https://opensource.org/licenses/MIT)

4. Compile the plugin DLL using Microsoft Visual Studio (on Windows), or GCC (on Linux)

5. Create a folder for your plugin, i.e. `vst2_bin/plugins/$(PLUGINNAME)/`

6. Duplicate the plugin DLL file and name them `plugin.dll.fx` and `plugin.dll.instr`

7. Copy the DLLs to `vst2_bin/plugins/$(PLUGINNAME)/`

8. Copy the plugin's assets (.svg files, readme, license, example patches) to `vst2_bin/plugins/$(PLUGINNAME)/`

9. Test and debug :-)


## Porting modules written for Rack v0.6

The steps are similar, here are some porting hints:

1. Do _not_ use static variables or members in your plugin. (Never ever !)

2. Use the `RACK_PLUGIN_MODEL_DECLARE`, `RACK_PLUGIN_INIT`, `RACK_PLUGIN_INIT_ID`, `RACK_PLUGIN_INIT_VERSION`, `RACK_PLUGIN_INIT_WEBSITE`, `RACK_PLUGIN_MODEL_ADD` macros (see example code for details)

3. Remove the `extern Plugin *plugin;` and `extern Model *modelXYZ;` declarations from the plugin's global include file

4. Add the following to your global plugin include file:
~~~
RACK_PLUGIN_DECLARE($(PLUGINNAME));

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "$(PLUGINNAME)"
#endif // USE_VST2
~~~

(the "plugin" define is mainly used to build asset path names, i.e. in `assetPlugin()` calls)

5. Rewrite the static modules initializations (`Model *model$(MODULENAME)> = Model::create<..>(..);`) as functions:
~~~
RACK_PLUGIN_MODEL_INIT($(PLUGINNAME), $(MODULENAME)) {
   Model *model = Model::create<$(MODULENAME), $(MODULENAME)Widget>($(PLUGINNAME), "$(MODULENAME)", "$(MODULEDISPLAYNAME)", ..);
   return model;
}
~~~

6. Replace all references to Rack internal global vars with macro calls:
     - `gRackScene` => `RACK_PLUGIN_UI_RACKSCENE`
     - `gRackWidget` => `RACK_PLUGIN_UI_RACKWIDGET`
     - `gToolbar` => `RACK_PLUGIN_UI_TOOLBAR`
     - `gDraggedWidget` => `RACK_PLUGIN_UI_DRAGGED_WIDGET`
     - `gHoveredWidget` => `RACK_PLUGIN_UI_HOVERED_WIDGET`
     - `gDragHoveredWidget` => `RACK_PLUGIN_UI_DRAGHOVERED_WIDGET`
     - `gFocusedWidget` => `RACK_PLUGIN_UI_FOCUSED_WIDGET` / `RACK_PLUGIN_UI_FOCUSED_WIDGET_SET`

7. On Windows: Fix any remaining MSVC compile errors (Rack modules were originally written for GCC and some use GCC-specific features like dynamically sized local arrays).


## Statically linked plugins

Statically linking a plugin to the VSVR VST is a bit trickier. You need to make sure that all linker symbols are unique, i.e. you may need to place the plugin code in its own namespace.
There are dozens of examples for this in the GitHub repo so please study them if you want to do that.

Otherwise stick to dynamically loaded plugins. They are much easier to handle and do not require the entire GitHub repo to be checked out / built. They also do _not_ require the VST2.4 SDK.


Happy Hacking !
