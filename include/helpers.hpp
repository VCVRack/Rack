#include "plugin/Model.hpp"
#include "ui/MenuLabel.hpp"
#include "ui/MenuItem.hpp"
#include "ui/Menu.hpp"
#include "app/PortWidget.hpp"
#include "app/ParamQuantity.hpp"
#include "app/ParamWidget.hpp"
#include "app/Scene.hpp"
#include "engine/Module.hpp"
#include "app.hpp"
#include "window.hpp"


namespace rack {


template <class TModule, class TModuleWidget, typename... Tags>
Model *createModel(std::string slug) {
	struct TModel : Model {
		Module *createModule() override {
			TModule *o = new TModule;
			return o;
		}
		ModuleWidget *createModuleWidget() override {
			TModule *module = new TModule;
			TModuleWidget *o = new TModuleWidget(module);
			o->model = this;
			return o;
		}
		ModuleWidget *createModuleWidgetNull() override {
			TModuleWidget *o = new TModuleWidget(NULL);
			o->model = this;
			return o;
		}
	};

	Model *o = new TModel;
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
	TWidget *o = new TWidget;
	o->box.pos = pos.minus(o->box.size.div(2));;
	return o;
}

template <class TParamWidget>
TParamWidget *createParam(math::Vec pos, Module *module, int paramId) {
	TParamWidget *o = new TParamWidget;
	o->box.pos = pos;
	if (module) {
		ParamQuantityFactory *f = module->paramInfos[paramId].paramQuantityFactory;
		if (f)
			o->paramQuantity = f->create();
		else
			o->paramQuantity = new ParamQuantity;
		o->paramQuantity->module = module;
		o->paramQuantity->paramId = paramId;
	}
	return o;
}

template <class TParamWidget>
TParamWidget *createParamCentered(math::Vec pos, Module *module, int paramId) {
	TParamWidget *o = createParam<TParamWidget>(pos, module, paramId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}

template <class TPortWidget>
TPortWidget *createInput(math::Vec pos, Module *module, int inputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos;
	o->module = module;
	o->type = PortWidget::INPUT;
	o->portId = inputId;
	return o;
}

template <class TPortWidget>
TPortWidget *createInputCentered(math::Vec pos, Module *module, int inputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos.minus(o->box.size.div(2));
	o->module = module;
	o->type = PortWidget::INPUT;
	o->portId = inputId;
	return o;
}

template <class TPortWidget>
TPortWidget *createOutput(math::Vec pos, Module *module, int outputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos;
	o->module = module;
	o->type = PortWidget::OUTPUT;
	o->portId = outputId;
	return o;
}

template <class TPortWidget>
TPortWidget *createOutputCentered(math::Vec pos, Module *module, int outputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos.minus(o->box.size.div(2));
	o->module = module;
	o->type = PortWidget::OUTPUT;
	o->portId = outputId;
	return o;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLight(math::Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *o = new TModuleLightWidget;
	o->box.pos = pos;
	o->module = module;
	o->firstLightId = firstLightId;
	return o;
}

template <class TModuleLightWidget>
TModuleLightWidget *createLightCentered(math::Vec pos, Module *module, int firstLightId) {
	TModuleLightWidget *o = new TModuleLightWidget;
	o->box.pos = pos.minus(o->box.size.div(2));
	o->module = module;
	o->firstLightId = firstLightId;
	return o;
}

template <class TMenuLabel = MenuLabel>
TMenuLabel *createMenuLabel(std::string text) {
	TMenuLabel *o = new TMenuLabel;
	o->text = text;
	return o;
}

template <class TMenuItem = MenuItem>
TMenuItem *createMenuItem(std::string text, std::string rightText = "") {
	TMenuItem *o = new TMenuItem;
	o->text = text;
	o->rightText = rightText;
	return o;
}

inline Menu *createMenu() {
	Menu *o = new Menu;
	o->box.pos = app()->window->mousePos;

	MenuOverlay *menuOverlay = new MenuOverlay;
	menuOverlay->addChild(o);

	app()->scene->addChild(menuOverlay);
	return o;
}


} // namespace rack
