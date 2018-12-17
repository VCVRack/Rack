#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"
#include "tags.hpp"
#include <list>


namespace rack {


struct ModuleWidget;
struct Module;


struct Model {
	Plugin *plugin = NULL;
	/** An identifier for the model, e.g. "VCO". Used for saving patches.
	The model slug must be unique in your plugin, but it doesn't need to be unique among different plugins.
	*/
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** The author name of the module.
	This might be different than the plugin slug. For example, if you create multiple plugins but want them to be branded similarly, you may use the same author in multiple plugins.
	You may even have multiple authors in one plugin, although this property will be moved to Plugin for Rack 1.0.
	*/
	std::string author;
	/** List of tags representing the function(s) of the module (optional) */
	std::list<ModelTag> tags;

	virtual ~Model() {}
	/** Creates a headless Module */
	virtual Module *createModule() { return NULL; }
	/** Creates a ModuleWidget with a Module attached */
	virtual ModuleWidget *createModuleWidget() { return NULL; }
	/** Creates a ModuleWidget with no Module, useful for previews */
	virtual ModuleWidget *createModuleWidgetNull() { return NULL; }
};


} // namespace rack
