#pragma once
#include <string>
#include <list>
#include "tags.hpp"


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

	/** Must be unique. Used for patch files and the VCV store API.
	To guarantee uniqueness, it is a good idea to prefix the slug by your "company name" if available, e.g. "MyCompany-MyPlugin"
	*/
	std::string slug;

	/** The version of your plugin
	Plugins should follow the versioning scheme described at https://github.com/VCVRack/Rack/issues/266
	Do not include the "v" in "v1.0" for example.
	*/
	std::string version;

	/** URL for plugin homepage (optional) */
	std::string website;
	/** URL for plugin manual (optional) */
	std::string manual;

	virtual ~Plugin();
	void addModel(Model *model);
};

struct Model {
	Plugin *plugin = NULL;
	/** An identifier for the model, e.g. "VCO". Used for saving patches. The slug, manufacturerSlug pair must be unique. */
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** The name of the manufacturer group of the module.
	This might be different than the plugin slug. For example, if you create multiple plugins but want them to be branded similarly, you may use the same manufacturer name in multiple plugins.
	You may even have multiple manufacturers in one plugin, although this would be unusual.
	*/
	std::string manufacturer;
	/** List of tags representing the function(s) of the module (optional) */
	std::list<ModelTag> tags;

	virtual ~Model() {}
	virtual ModuleWidget *createModuleWidget() { return NULL; }
};

void pluginInit();
void pluginDestroy();
void pluginLogIn(std::string email, std::string password);
void pluginLogOut();
/** Returns whether a new plugin is available, and downloads it unless doing a dry run */
bool pluginSync(bool dryRun);
void pluginCancelDownload();
bool pluginIsLoggedIn();
bool pluginIsDownloading();
float pluginGetDownloadProgress();
std::string pluginGetDownloadName();
std::string pluginGetLoginStatus();


extern std::list<Plugin*> gPlugins;
extern std::string gToken;


} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

/** Called once to initialize and return the Plugin instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::Plugin *plugin);
