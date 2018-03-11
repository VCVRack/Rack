#pragma once

// Include headers that plugins will likely use, for convenience
#include "util/common.hpp"
#include "asset.hpp"
#include "plugin.hpp"
#include "engine.hpp"
#include "widgets.hpp"
#include "app.hpp"
#include "ui.hpp"
#include "componentlibrary.hpp"


namespace rack {


////////////////////
// helpers
////////////////////

/** Deprecated, use Model::create<TModule, TModuleWidget>(...) instead */
template <class TModuleWidget, typename... Tags>
DEPRECATED Model *createModel(std::string author, std::string slug, std::string name, Tags... tags) {
	struct TModel : Model {
		ModuleWidget *createModuleWidget() override {
			ModuleWidget *moduleWidget = new TModuleWidget();
			moduleWidget->model = this;
			return moduleWidget;
		}
	};
	Model *model = new TModel();
	model->author = author;
	model->slug = slug;
	model->name = name;
	model->tags = {tags...};
	return model;
}

/** Deprecated, use Widget::create<TScrew>() instead */
template <class TScrew>
DEPRECATED TScrew *createScrew(Vec pos) {
	TScrew *screw = new TScrew();
	screw->box.pos = pos;
	return screw;
}

/** Deprecated, use ParamWidget::create<TParamWidget>() instead */
template <class TParamWidget>
DEPRECATED TParamWidget *createParam(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	TParamWidget *param = new TParamWidget();
	param->box.pos = pos;
	param->module = module;
	param->paramId = paramId;
	param->setLimits(minValue, maxValue);
	param->setDefaultValue(defaultValue);
	return param;
}

/** Deprecated, use Port::create<TPort>(..., Port::INPUT, ...) instead */
template <class TPort>
DEPRECATED TPort *createInput(Vec pos, Module *module, int inputId) {
	TPort *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	return port;
}

/** Deprecated, use Port::create<TPort>(..., Port::OUTPUT, ...) instead */
template <class TPort>
DEPRECATED TPort *createOutput(Vec pos, Module *module, int outputId) {
	TPort *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = outputId;
	return port;
}

/** Deprecated, use ModuleLightWidget::create<TModuleLightWidget>() instead */
template<class TModuleLightWidget>
DEPRECATED TModuleLightWidget *createLight(Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = firstLightId;
	return light;
}


} // namespace rack
