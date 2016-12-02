#pragma once

#include <string>
#include <list>
#include <vector>
#include <memory>
#include "widgets.hpp"


namespace rack {

extern Scene *gScene;
extern RackWidget *gRackWidget;

////////////////////
// Plugin manager
////////////////////

struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	virtual ~Plugin();

	// A unique identifier for your plugin, e.g. "foo"
	std::string slug;
	// Human readable name for your plugin, e.g. "Foo Modular"
	std::string name;
	// A list of the models made available by this plugin
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

////////////////////
// gui.cpp
////////////////////

extern Vec gMousePos;
extern Widget *gHoveredWidget;
extern Widget *gDraggedWidget;

void guiInit();
void guiDestroy();
void guiRun();
void guiCursorLock();
void guiCursorUnlock();

int loadFont(std::string filename);
int loadImage(std::string filename);
void drawImage(NVGcontext *vg, Vec pos, int imageId);

////////////////////
// rack.cpp
////////////////////

// TODO Find a clean way to make this a variable
#define SAMPLE_RATE 44100


struct Wire;

struct Module {
	std::vector<float> params;
	// Pointers to voltage values at each port
	// If value is NULL, the input/output is disconnected
	std::vector<float*> inputs;
	std::vector<float*> outputs;

	virtual ~Module() {}

	// Always called on each sample frame before calling getOutput()
	virtual void step() {}
};

struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	// The voltage which is pointed to by module inputs/outputs
	float value = 0.0;
};

void rackInit();
void rackDestroy();
void rackStart();
void rackStop();
// Does not transfer ownership
void rackAddModule(Module *module);
void rackRemoveModule(Module *module);
// Does not transfer ownership
void rackConnectWire(Wire *wire);
void rackDisconnectWire(Wire *wire);
void rackSetParamSmooth(Module *module, int paramId, float value);

////////////////////
// Optional helpers for plugins
////////////////////

inline
Plugin *createPlugin(std::string slug, std::string name) {
	Plugin *plugin = new Plugin();
	plugin->slug = slug;
	plugin->name = name;
	return plugin;
}

template <class TModuleWidget>
Model *createModel(Plugin *plugin, std::string slug, std::string name) {
	struct TModel : Model {
		ModuleWidget *createModuleWidget() {
			ModuleWidget *moduleWidget = new TModuleWidget();
			moduleWidget->model = this;
			return moduleWidget;
		}
	};
	Model *model = new TModel();
	model->plugin = plugin;
	model->slug = slug;
	model->name = name;
	// Create bi-directional association between the Plugin and Model
	if (plugin) {
		plugin->models.push_back(model);
	}
	return model;
}

template <class TParam>
ParamWidget *createParam(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	ParamWidget *param = new TParam();
	param->box.pos = pos;
	param->module = module;
	param->paramId = paramId;
	param->setLimits(minValue, maxValue);
	param->setDefaultValue(defaultValue);
	return param;
}

inline
InputPort *createInput(Vec pos, Module *module, int inputId) {
	InputPort *port = new InputPort();
	port->box.pos = pos;
	port->module = module;
	port->inputId = inputId;
	return port;
}

inline
OutputPort *createOutput(Vec pos, Module *module, int outputId) {
	OutputPort *port = new OutputPort();
	port->box.pos = pos;
	port->module = module;
	port->outputId = outputId;
	return port;
}

inline
Screw *createScrew(Vec pos) {
	Screw *screw = new Screw();
	screw->box.pos = pos;
	return screw;
}


} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

// Called once to initialize and return Plugin.
// Plugin is destructed when Rack closes
extern "C"
rack::Plugin *init();
