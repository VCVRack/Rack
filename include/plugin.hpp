#pragma once
#include <string>
#include <list>


namespace rack {


struct ModuleWidget;
struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	virtual ~Plugin();

	// A unique identifier for your plugin, e.g. "foo"
	std::string slug;
	// Human readable name for your plugin, e.g. "Foo Modular"
	std::string name;
	/** A list of the models made available by this plugin */
	std::list<Model*> models;
};

struct Model {
	virtual ~Model() {}

	Plugin *plugin;
	// A unique identifier for the model in this plugin, e.g. "vco"
	std::string slug;
	// Human readable name for your model, e.g. "VCO"
	std::string name;
	virtual ModuleWidget *createModuleWidget() { return NULL; }
};

extern std::list<Plugin*> gPlugins;

void pluginInit();
void pluginDestroy();

void pluginLogIn(std::string email, std::string password);
void pluginLogOut();
void pluginRefresh();
void pluginCancelDownload();
bool pluginIsLoggedIn();
bool pluginIsDownloading();
float pluginGetDownloadProgress();
std::string pluginGetDownloadName();
std::string pluginGetLoginStatus();

} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

/** Called once to initialize and return Plugin.
Plugin is destructed when Rack closes
*/
extern "C"
rack::Plugin *init();
