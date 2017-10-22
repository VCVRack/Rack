#pragma once
#include <string>
#include <list>


namespace rack {


struct ModuleWidget;
struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Manufacturer {
	/** A list of the models available by this manufacturer, add with addModel() */
	std::list<Model*> models;
	/** The file path of the plugins directory */
	std::string path;
	/** OS-dependent library handle */
	void *handle = NULL;

	// You may set everything below this point in your plugin

	/** A unique identifier for the manufacturer, e.g. "foo" */
	std::string slug;
	/** Human readable name for the manufacturer, e.g. "Foo Modular" */
	std::string name;
	/** Optional metadata for the Add Module context menu */
	std::string homepageUrl;
	std::string manualUrl;
	std::string version;

	virtual ~Manufacturer();
	void addModel(Model *model);
};

struct Model {
	Manufacturer *manufacturer = NULL;
	/** A unique identifier for the model, e.g. "VCO" */
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;

	virtual ~Model() {}
	virtual ModuleWidget *createModuleWidget() { return NULL; }
};

extern std::list<Manufacturer*> gManufacturers;
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

/** Called once to initialize and return the Manufacturer instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::Manufacturer *manufacturer);
