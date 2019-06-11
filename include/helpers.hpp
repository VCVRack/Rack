#pragma once
#include <plugin/Model.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuLabel.hpp>
#include <ui/Menu.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <app/Scene.hpp>
#include <engine/Module.hpp>
#include <engine/ParamQuantity.hpp>
#include <app.hpp>
#include <window.hpp>


namespace rack {


template <class TModule, class TModuleWidget, typename... Tags>
plugin::Model *createModel(const std::string &slug) {
	struct TModel : plugin::Model {
		engine::Module *createModule() override {
			engine::Module *m = new TModule;
			m->model = this;
			return m;
		}
		app::ModuleWidget *createModuleWidget() override {
			TModule *m = new TModule;
			m->engine::Module::model = this;
			app::ModuleWidget *mw = new TModuleWidget(m);
			mw->model = this;
			return mw;
		}
		app::ModuleWidget *createModuleWidgetNull() override {
			app::ModuleWidget *mw = new TModuleWidget(NULL);
			mw->model = this;
			return mw;
		}
	};

	plugin::Model *o = new TModel;
	o->slug = slug;
	return o;
}

template <class TWidget>
TWidget *createWidget(math::Vec pos) {
	TWidget *o = new TWidget;
	o->box.pos = pos;
	return o;
}

template <class TWidget>
TWidget *createWidgetCentered(math::Vec pos) {
	TWidget *o = createWidget<TWidget>(pos);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TParamWidget>
TParamWidget *createParam(math::Vec pos, engine::Module *module, int paramId) {
	TParamWidget *o = new TParamWidget;
	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
	}
	return o;
}

template <class TParamWidget>
TParamWidget *createParamCentered(math::Vec pos, engine::Module *module, int paramId) {
	TParamWidget *o = createParam<TParamWidget>(pos, module, paramId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TPortWidget>
TPortWidget *createInput(math::Vec pos, engine::Module *module, int inputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos;
	o->module = module;
	o->type = app::PortWidget::INPUT;
	o->portId = inputId;
	return o;
}

template <class TPortWidget>
TPortWidget *createInputCentered(math::Vec pos, engine::Module *module, int inputId) {
	TPortWidget *o = createInput<TPortWidget>(pos, module, inputId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TPortWidget>
TPortWidget *createOutput(math::Vec pos, engine::Module *module, int outputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos;
	o->module = module;
	o->type = app::PortWidget::OUTPUT;
	o->portId = outputId;
	return o;
}

template <class TPortWidget>
TPortWidget *createOutputCentered(math::Vec pos, engine::Module *module, int outputId) {
	TPortWidget *o = createOutput<TPortWidget>(pos, module, outputId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLight(math::Vec pos, engine::Module *module, int firstLightId) {
	TModuleLightWidget *o = new TModuleLightWidget;
	o->box.pos = pos;
	o->module = module;
	o->firstLightId = firstLightId;
	return o;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLightCentered(math::Vec pos, engine::Module *module, int firstLightId) {
	TModuleLightWidget *o = createLight<TModuleLightWidget>(pos, module, firstLightId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TMenuLabel = ui::MenuLabel>
TMenuLabel *createMenuLabel(std::string text) {
	TMenuLabel *o = new TMenuLabel;
	o->text = text;
	return o;
}

template <class TMenuItem = ui::MenuItem>
TMenuItem *createMenuItem(std::string text, std::string rightText = "") {
	TMenuItem *o = new TMenuItem;
	o->text = text;
	o->rightText = rightText;
	return o;
}

template <class TMenu = ui::Menu>
TMenu *createMenu() {
	TMenu *o = new TMenu;
	o->box.pos = APP->window->mousePos;

	ui::MenuOverlay *menuOverlay = new ui::MenuOverlay;
	menuOverlay->addChild(o);

	APP->scene->addChild(menuOverlay);
	return o;
}


} // namespace rack
