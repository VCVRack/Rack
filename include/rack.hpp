#pragma once

#include "util.hpp"
#include "math.hpp"
#include "asset.hpp"
#include "plugin.hpp"
#include "engine.hpp"
#include "gui.hpp"
#include "app.hpp"
#include "components.hpp"
#include <iostream>

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
            
            // TODO move node creation here
            
            node = &ossia::net::create_node(rack::root_dev(),name);
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
Widget *createScrew(Vec pos) {
	Widget *screw = new TScrew();
	screw->box.pos = pos;
	return screw;
}

template <class TParamWidget>
ParamWidget *createParam(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	ParamWidget *param = new TParamWidget();
	param->box.pos = pos;
	param->module = module;
	param->paramId = paramId;
    
    auto& p = module->params[paramId];
    auto& p_node = ossia::net::create_node(*module->node,p.name);
    p.ossia_param = p_node.create_parameter(ossia::val_type::FLOAT);
    p.ossia_param->set_domain(ossia::make_domain(minValue,maxValue));
    p.ossia_param->set_bounding(ossia::bounding_mode::CLIP);
    p.ossia_param->push_value(defaultValue);
    p.ossia_param->set_default_value(defaultValue);
    
    p.ossia_param->add_callback([param] (const ossia::value& v) {
        auto& p = param->module->params[param->paramId];
        param->value = v.get<float>();
        p.value = param->value;
        if ( auto fbw = dynamic_cast<FramebufferWidget*>(param))
            fbw->dirty = true;
    });
    
    param->setLimits(minValue, maxValue);
	param->setDefaultValue(defaultValue);
    
	return param;
}

template <class TPort>
Port *createInput(Vec pos, Module *module, int inputId) {
	Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	return port;
}

template <class TPort>
Port *createOutput(Vec pos, Module *module, int outputId) {
	Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = outputId;
	return port;
}

template<class TModuleLightWidget>
ModuleLightWidget *createLight(Vec pos, Module *module, int firstLightId) {
	ModuleLightWidget *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = firstLightId;
	return light;
}


} // namespace rack
