#pragma once
#include "plugin.hpp"
#include "engine.hpp"
#include "gui.hpp"
#include "components.hpp"


namespace rack {


////////////////////
// helpers
////////////////////

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

template <class TScrew>
Screw *createScrew(Vec pos) {
	Screw *screw = new TScrew();
	screw->box.pos = pos;
	return screw;
}

template <class TLight>
ValueLight *createValueLight(Vec pos, float *value) {
	ValueLight *light = new TLight();
	light->box.pos = pos;
	light->value = value;
	return light;
}


} // namespace rack
