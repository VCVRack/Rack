#include "plugin.hpp"
#include "engine.hpp"
#include "app.hpp"


namespace rack {


template <class TModule, class TModuleWidget, typename... Tags>
Model *createModel(std::string author, std::string slug, std::string name, Tags... tags) {
	struct TModel : Model {
		Module *createModule() override {
			TModule *module = new TModule();
			return module;
		}
		ModuleWidget *createModuleWidget() override {
			TModule *module = new TModule();
			TModuleWidget *moduleWidget = new TModuleWidget(module);
			moduleWidget->model = this;
			return moduleWidget;
		}
		ModuleWidget *createModuleWidgetNull() override {
			TModuleWidget *moduleWidget = new TModuleWidget(NULL);
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

template <class TWidget>
TWidget *createWidget(Vec pos) {
	TWidget *w = new TWidget();
	w->box.pos = pos;
	return w;
}

/** Deprecated. Use createWidget<TScrew>() instead */
template <class TScrew>
DEPRECATED TScrew *createScrew(Vec pos) {
	return createWidget<TScrew>(pos);
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

template <class TParamWidget>
TParamWidget *createParamCentered(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	TParamWidget *param = new TParamWidget();
	param->box.pos = pos.minus(param->box.size.div(2));
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
TPort *createInputCentered(Vec pos, Module *module, int inputId) {
	TPort *port = new TPort();
	port->box.pos = pos.minus(port->box.size.div(2));
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

template <class TPort>
TPort *createOutputCentered(Vec pos, Module *module, int outputId) {
	TPort *port = new TPort();
	port->box.pos = pos.minus(port->box.size.div(2));
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = outputId;
	return port;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLight(Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = firstLightId;
	return light;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLightCentered(Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *light = new TModuleLightWidget();
	light->box.pos = pos.minus(light->box.size.div(2));
	light->module = module;
	light->firstLightId = firstLightId;
	return light;
}


} // namespace rack
