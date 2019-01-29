#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"
#include <jansson.h>
#include <list>


namespace rack {


namespace app {
	struct ModuleWidget;
} // namespace app

struct Module;


struct Model {
	Plugin *plugin = NULL;

	/** An identifier for the model, e.g. "VCO". Used for saving patches.
	The model slug must be unique in your plugin, but it doesn't need to be unique among different plugins.
	*/
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** A one-line summary of the module's purpose */
	std::string description;
	/** List of tags representing the function(s) of the module */
	std::list<std::string> tags;

	virtual ~Model() {}
	/** Creates a headless Module */
	virtual Module *createModule() { return NULL; }
	/** Creates a ModuleWidget with a Module attached */
	virtual app::ModuleWidget *createModuleWidget() { return NULL; }
	/** Creates a ModuleWidget with no Module, useful for previews */
	virtual app::ModuleWidget *createModuleWidgetNull() { return NULL; }

	void fromJson(json_t *rootJ);
};


} // namespace rack
