#pragma once

#include <string>
#include <list>
#include <vector>
#include <memory>
#include <set>
#include <thread>
#include <mutex>
#include "rackwidgets.hpp"


namespace rack {

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
extern Widget *gSelectedWidget;
extern int gGuiFrame;

void guiInit();
void guiDestroy();
void guiRun();
void guiCursorLock();
void guiCursorUnlock();
const char *guiSaveDialog(const char *filters, const char *filename);
const char *guiOpenDialog(const char *filters, const char *filename);

int loadFont(std::string filename);
int loadImage(std::string filename);
void drawImage(NVGcontext *vg, Vec pos, int imageId);

////////////////////
// rack.cpp
////////////////////

struct Wire;

struct Module {
	std::vector<float> params;
	/** Pointers to voltage values at each port
	If value is NULL, the input/output is disconnected
	*/
	std::vector<float*> inputs;
	std::vector<float*> outputs;
	/** For CPU usage */
	float cpuTime = 0.0;

	virtual ~Module() {}

	// Always called on each sample frame before calling getOutput()
	virtual void step() {}
};

struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	/** The voltage connected to input ports */
	float inputValue = 0.0;
	/** The voltage connected to output ports */
	float outputValue = 0.0;
};

struct Rack {
	Rack();
	~Rack();
	/** Launches rack thread */
	void start();
	void stop();
	void run();
	void step();
	/** Does not transfer pointer ownership */
	void addModule(Module *module);
	void removeModule(Module *module);
	/** Does not transfer pointer ownership */
	void addWire(Wire *wire);
	void removeWire(Wire *wire);
	void setParamSmooth(Module *module, int paramId, float value);

	float sampleRate;

	struct Impl;
	Impl *impl;
};

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

template <class TInputPort>
InputPort *createInput(Vec pos, Module *module, int inputId) {
	InputPort *port = new TInputPort();
	port->box.pos = pos;
	port->module = module;
	port->inputId = inputId;
	return port;
}

template <class TOutputPort>
OutputPort *createOutput(Vec pos, Module *module, int outputId) {
	OutputPort *port = new TOutputPort();
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

////////////////////
// Globals
////////////////////

extern std::string gApplicationName;
extern std::string gApplicationVersion;

extern Scene *gScene;
extern RackWidget *gRackWidget;

extern Rack *gRack;

} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

// Called once to initialize and return Plugin.
// Plugin is destructed when Rack closes
extern "C"
rack::Plugin *init();
