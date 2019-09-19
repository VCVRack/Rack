#pragma once
#include <common.hpp>
#include <plugin/Plugin.hpp>
#include <jansson.h>
#include <set>


namespace rack {


namespace app {
struct ModuleWidget;
} // namespace app


namespace engine {
struct Module;
} // namespace engine


namespace plugin {


struct Model {
	Plugin* plugin = NULL;
	std::vector<std::string> presetPaths;

	/** Must be unique. Used for saving patches. Never change this after releasing your module.
	The model slug must be unique within your plugin, but it doesn't need to be unique among different plugins.
	*/
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** List of tag IDs representing the function(s) of the module.
	Tag IDs are not part of the ABI and may change at any time.
	*/
	std::vector<int> tags;
	/** A one-line summary of the module's purpose */
	std::string description;

	virtual ~Model() {}
	/** Creates a headless Module */
	virtual engine::Module* createModule() {
		return NULL;
	}
	/** Creates a ModuleWidget with a Module attached */
	virtual app::ModuleWidget* createModuleWidget() {
		return NULL;
	}
	/** Creates a ModuleWidget with no Module, useful for previews */
	virtual app::ModuleWidget* createModuleWidgetNull() {
		return NULL;
	}

	void fromJson(json_t* rootJ);
};


} // namespace plugin
} // namespace rack
