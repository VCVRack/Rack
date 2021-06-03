#pragma once
#include <plugin/Model.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuLabel.hpp>
#include <ui/Menu.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/Scene.hpp>
#include <app/SvgPanel.hpp>
#include <engine/Module.hpp>
#include <engine/ParamQuantity.hpp>
#include <context.hpp>

#include <functional>


namespace rack {


template <class TModule, class TModuleWidget>
plugin::Model* createModel(const std::string& slug) {
	struct TModel : plugin::Model {
		engine::Module* createModule() override {
			engine::Module* m = new TModule;
			m->model = this;
			return m;
		}
		app::ModuleWidget* createModuleWidget(engine::Module* m) override {
			TModule* tm = NULL;
			if (m) {
				assert(m->model == this);
				tm = dynamic_cast<TModule*>(m);
			}
			app::ModuleWidget* mw = new TModuleWidget(tm);
			mw->model = this;
			return mw;
		}
	};

	plugin::Model* o = new TModel;
	o->slug = slug;
	return o;
}


template <class TWidget>
TWidget* createWidget(math::Vec pos) {
	TWidget* o = new TWidget;
	o->box.pos = pos;
	return o;
}


template <class TWidget>
TWidget* createWidgetCentered(math::Vec pos) {
	TWidget* o = createWidget<TWidget>(pos);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


inline app::SvgPanel* createPanel(std::string svgPath) {
	app::SvgPanel* panel = new app::SvgPanel;
	std::shared_ptr<Svg> svg = Svg::load(svgPath);
	panel->setBackground(svg);
	return panel;
}


template <class TParamWidget>
TParamWidget* createParam(math::Vec pos, engine::Module* module, int paramId) {
	TParamWidget* o = new TParamWidget;
	o->box.pos = pos;
	o->app::ParamWidget::module = module;
	o->app::ParamWidget::paramId = paramId;
	o->initParamQuantity();
	return o;
}


template <class TParamWidget>
TParamWidget* createParamCentered(math::Vec pos, engine::Module* module, int paramId) {
	TParamWidget* o = createParam<TParamWidget>(pos, module, paramId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


template <class TPortWidget>
TPortWidget* createInput(math::Vec pos, engine::Module* module, int inputId) {
	TPortWidget* o = new TPortWidget;
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
	return o;
}


template <class TPortWidget>
TPortWidget* createInputCentered(math::Vec pos, engine::Module* module, int inputId) {
	TPortWidget* o = createInput<TPortWidget>(pos, module, inputId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


template <class TPortWidget>
TPortWidget* createOutput(math::Vec pos, engine::Module* module, int outputId) {
	TPortWidget* o = new TPortWidget;
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
	return o;
}


template <class TPortWidget>
TPortWidget* createOutputCentered(math::Vec pos, engine::Module* module, int outputId) {
	TPortWidget* o = createOutput<TPortWidget>(pos, module, outputId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


template <class TModuleLightWidget>
TModuleLightWidget* createLight(math::Vec pos, engine::Module* module, int firstLightId) {
	TModuleLightWidget* o = new TModuleLightWidget;
	o->box.pos = pos;
	o->app::ModuleLightWidget::module = module;
	o->app::ModuleLightWidget::firstLightId = firstLightId;
	return o;
}


template <class TModuleLightWidget>
TModuleLightWidget* createLightCentered(math::Vec pos, engine::Module* module, int firstLightId) {
	TModuleLightWidget* o = createLight<TModuleLightWidget>(pos, module, firstLightId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


/** Creates a param with a light and calls setFirstLightId() on it. */
template <class TParamWidget>
TParamWidget* createLightParam(math::Vec pos, engine::Module* module, int paramId, int firstLightId) {
	TParamWidget* o = createParam<TParamWidget>(pos, module, paramId);
	o->setFirstLightId(firstLightId);
	return o;
}


template <class TParamWidget>
TParamWidget* createLightParamCentered(math::Vec pos, engine::Module* module, int paramId, int firstLightId) {
	TParamWidget* o = createLightParam<TParamWidget>(pos, module, paramId, firstLightId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


template <class TMenuLabel = ui::MenuLabel>
TMenuLabel* createMenuLabel(std::string text) {
	TMenuLabel* o = new TMenuLabel;
	o->text = text;
	return o;
}


template <class TMenuItem = ui::MenuItem>
TMenuItem* createMenuItem(std::string text, std::string rightText = "") {
	TMenuItem* o = new TMenuItem;
	o->text = text;
	o->rightText = rightText;
	return o;
}


template <class TMenu = ui::Menu>
TMenu* createMenu() {
	TMenu* o = new TMenu;
	o->box.pos = APP->scene->mousePos;

	ui::MenuOverlay* menuOverlay = new ui::MenuOverlay;
	menuOverlay->addChild(o);

	APP->scene->addChild(menuOverlay);
	return o;
}


/** Creates a MenuItem that controls a boolean value with a check mark.
*/
inline ui::MenuItem* createBoolMenuItem(std::string name, std::function<bool()> getter, std::function<void(bool)> setter) {
	struct Item : ui::MenuItem {
		std::function<void(size_t)> setter;
		bool val;

		void onAction(const event::Action& e) override {
			setter(val);
		}
	};

	bool currVal = getter();
	Item* item = createMenuItem<Item>(name, CHECKMARK(currVal));
	item->setter = setter;
	item->val = !currVal;
	return item;
}


/** Easy wrapper for createBoolMenuItem() to modify a bool pointer.
*/
template <typename T>
ui::MenuItem* createBoolPtrMenuItem(std::string name, T* ptr) {
	return createBoolMenuItem(name,
		[=]() {return *ptr;},
		[=](T val) {*ptr = val;}
	);
}


/** Creates a MenuItem that when hovered, opens a submenu with several MenuItems indexed by an integer.
*/
inline ui::MenuItem* createIndexSubmenuItem(std::string name, std::vector<std::string> labels, std::function<size_t()> getter, std::function<void(size_t)> setter) {
	struct IndexItem : ui::MenuItem {
		std::function<void(size_t)> setter;
		size_t index;

		void onAction(const event::Action& e) override {
			setter(index);
		}
	};

	struct Item : ui::MenuItem {
		std::function<size_t()> getter;
		std::function<void(size_t)> setter;
		std::vector<std::string> labels;

		ui::Menu* createChildMenu() override {
			ui::Menu* menu = new ui::Menu;
			size_t currIndex = getter();
			for (size_t i = 0; i < labels.size(); i++) {
				IndexItem* item = createMenuItem<IndexItem>(labels[i], CHECKMARK(currIndex == i));
				item->setter = setter;
				item->index = i;
				menu->addChild(item);
			}
			return menu;
		}
	};

	size_t currIndex = getter();
	std::string label = (currIndex < labels.size()) ? labels[currIndex] : "";
	Item* item = createMenuItem<Item>(name, label + "  " + RIGHT_ARROW);
	item->getter = getter;
	item->setter = setter;
	item->labels = labels;
	return item;
}


/** Easy wrapper for createIndexSubmenuItem() that controls an integer index at a pointer address.
*/
template <typename T>
ui::MenuItem* createIndexPtrSubmenuItem(std::string name, std::vector<std::string> labels, T* ptr) {
	return createIndexSubmenuItem(name, labels,
		[=]() {return *ptr;},
		[=](T index) {*ptr = index;}
	);
}


} // namespace rack
