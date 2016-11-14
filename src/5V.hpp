#pragma once

#include <string>
#include <list>
#include <memory>
#include "widgets.hpp"
#include "rack.hpp"


extern Scene *gScene;
extern RackWidget *gRackWidget;

////////////////////
// Plugin manager
////////////////////

struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	virtual ~Plugin();

	// A unique identifier for your plugin, e.g. "simple"
	std::string slug;
	// Human readable name for your plugin, e.g. "Simple Modular"
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
// midi.cpp
////////////////////

void midiInit();
void midiDestroy();
int midiPortCount();
std::string midiPortName(int portId);
void midiPortOpen(int portId);
void midiPortClose();

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
long rackGetFrame();
void rackRequestFrame(long frame);
void rackSetParamSmooth(Module *module, int paramId, float value);

////////////////////
// Implemented by plugin
////////////////////

// Called once to initialize and return Plugin.
// Plugin is destructed by the 5V engine when it closes
extern "C"
Plugin *init();

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
ParamWidget *createParamWidget(ModuleWidget *moduleWidget, int paramId, float minValue, float maxValue, float defaultValue, Vec pos) {
	ParamWidget *param = new TParam();
	param->moduleWidget = moduleWidget;
	param->paramId = paramId;
	param->setLimits(minValue, maxValue);
	param->setDefaultValue(defaultValue);
	param->box.pos = pos;
	// Create bi-directional association between the Param and ModelWidget
	moduleWidget->params[paramId] = param;
	moduleWidget->addChild(param);
	return param;
}

inline
InputPort *createInputPort(ModuleWidget *moduleWidget, int inputId, Vec pos) {
	InputPort *port = new InputPort();
	port->moduleWidget = moduleWidget;
	port->inputId = inputId;
	port->box.pos = pos;
	// Create bi-directional association between the InputPort and ModelWidget
	moduleWidget->inputs[inputId] = port;
	moduleWidget->addChild(port);
	return port;
}

inline
OutputPort *createOutputPort(ModuleWidget *moduleWidget, int outputId, Vec pos) {
	OutputPort *port = new OutputPort();
	port->moduleWidget = moduleWidget;
	port->outputId = outputId;
	port->box.pos = pos;
	// Create bi-directional association between the OutputPort and ModelWidget
	moduleWidget->outputs[outputId] = port;
	moduleWidget->addChild(port);
	return port;
}

inline
Screw *createScrew(ModuleWidget *moduleWidget, Vec pos) {
	Screw *screw = new Screw();
	screw->box.pos = pos;
	moduleWidget->addChild(screw);
	return screw;
}
