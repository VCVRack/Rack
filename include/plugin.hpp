#pragma once
#include <string>
#include <list>


namespace rack {


struct ModuleWidget;
struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	/** A list of the models available by this plugin, add with addModel() */
	std::list<Model*> models;
	/** The file path of the plugin's directory */
	std::string path;
	/** OS-dependent library handle */
	void *handle = NULL;

	/** Used when syncing plugins with the API */
	std::string slug;
	/** The version of your plugin
	Plugins should follow the versioning scheme described at https://github.com/VCVRack/Rack/issues/266
	Do not include the "v" in "v1.0" for example.
	*/
	std::string version;

	virtual ~Plugin();
	void addModel(Model *model);
};

struct Model {
	Plugin *plugin = NULL;
	/** An identifier for the model, e.g. "VCO". Used for saving patches. The slug, manufacturerSlug pair must be unique. */
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** An identifier for the manufacturer, e.g. "foo". Used for saving patches. */
	std::string manufacturerSlug;
	/** Human readable name for the manufacturer, e.g. "Foo Modular" */
	std::string manufacturerName;

	virtual ~Model() {}
	virtual ModuleWidget *createModuleWidget() { return NULL; }
};

extern std::list<Plugin*> gPlugins;
extern std::string gToken;

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

/** Called once to initialize and return the Plugin instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::Plugin *plugin);
