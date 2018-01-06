#pragma once

#include "util.hpp"
#include "math.hpp"
#include "asset.hpp"
#include "plugin.hpp"
#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "componentlibrary.hpp"


namespace rack {


////////////////////
// helpers
////////////////////


template <class TModuleWidget, typename... Tags>
Model *createModel(std::string manufacturer, std::string slug, std::string name, Tags... tags) {
	struct TModel : Model {
		ModuleWidget *createModuleWidget() override {
			ModuleWidget *moduleWidget = new TModuleWidget();
			moduleWidget->model = this;
			return moduleWidget;
		}
	};
	Model *model = new TModel();
	model->manufacturer = manufacturer;
	model->slug = slug;
	model->name = name;
	model->tags = {tags...};
	return model;
}

template <class TScrew>
TScrew *createScrew(Vec pos) {
	TScrew *screw = new TScrew();
	screw->box.pos = pos;
	return screw;
}

template <class TParamWidget>
TParamWidget *createParam(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	TParamWidget *param = new TParamWidget();
	param->box.pos = pos;
	param->module = module;
	param->paramId = paramId;
	param->setLimits(minValue, maxValue);
	param->setDefaultValue(defaultValue);
	return param;
}

template <class TPort>
TPort *createInput(Vec pos, Module *module, int inputId) {
	TPort *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	return port;
}

template <class TPort>
TPort *createOutput(Vec pos, Module *module, int outputId) {
	TPort *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = outputId;
	return port;
}

template<class TModuleLightWidget>
TModuleLightWidget *createLight(Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = firstLightId;
	return light;
}


} // namespace rack
