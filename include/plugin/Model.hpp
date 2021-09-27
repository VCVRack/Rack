#pragma once
#include <common.hpp>
#include <plugin/Plugin.hpp>

#include <jansson.h>

#include <list>


namespace rack {


namespace ui {
struct Menu;
} // namespace app


namespace app {
struct ModuleWidget;
} // namespace app


namespace engine {
struct Module;
} // namespace engine


namespace plugin {


/** Type information for a module.
Factory for Module and ModuleWidget.
*/
struct Model {
	Plugin* plugin = NULL;

	/** Must be unique. Used for saving patches. Never change this after releasing your module.
	The model slug must be unique within your plugin, but it doesn't need to be unique among different plugins.
	*/
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** List of tag IDs representing the function(s) of the module.
	Tag IDs are not part of the ABI and may change at any time.
	*/
	std::list<int> tagIds;
	/** A one-line summary of the module's purpose */
	std::string description;
	/** The manual of the module. HTML, PDF, or GitHub readme/wiki are fine.
	*/
	std::string manualUrl;
	std::string modularGridUrl;

	/** Hides model from the Module Browser but able to be loaded from a patch file.
	Useful for deprecating modules without breaking old patches.
	*/
	bool hidden = false;

	virtual ~Model() {}
	/** Creates a Module. */
	virtual engine::Module* createModule() {
		return NULL;
	}
	/** Creates a ModuleWidget with a Module attached.
	Module may be NULL.
	*/
	virtual app::ModuleWidget* createModuleWidget(engine::Module* m) {
		return NULL;
	}

	void fromJson(json_t* rootJ);
	/** Returns the branded name of the model, e.g. VCV VCO-1. */
	std::string getFullName();
	std::string getFactoryPresetDirectory();
	std::string getUserPresetDirectory();
	/** Returns the module or plugin manual URL, whichever exists. */
	std::string getManualUrl();

	/** Appends items to menu with useful Model information.

	Enable `inBrowser` to show Module Browser key commands.
	*/
	void appendContextMenu(ui::Menu* menu, bool inBrowser = false);
	bool isFavorite();
	void setFavorite(bool favorite);
};


} // namespace plugin
} // namespace rack
