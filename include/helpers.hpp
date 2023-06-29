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


/** Returns a Model that constructs a Module and ModuleWidget subclass. */
template <class TModule, class TModuleWidget>
plugin::Model* createModel(std::string slug) {
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
			assert(mw->module == m);
			mw->setModel(this);
			return mw;
		}
	};

	plugin::Model* o = new TModel;
	o->slug = slug;
	return o;
}


/** Creates a Widget subclass with its top-left at a position. */
template <class TWidget>
TWidget* createWidget(math::Vec pos) {
	TWidget* o = new TWidget;
	o->box.pos = pos;
	return o;
}


/** Creates a Widget subclass with its center at a position. */
template <class TWidget>
TWidget* createWidgetCentered(math::Vec pos) {
	TWidget* o = createWidget<TWidget>(pos);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


/** Creates an SvgPanel and loads the SVG from the given path. */
template <class TPanel = app::SvgPanel>
TPanel* createPanel(std::string svgPath) {
	TPanel* panel = new TPanel;
	panel->setBackground(window::Svg::load(svgPath));
	return panel;
}


/** Creates a ThemedSvgPanel and loads the light/dark SVGs from the given paths. */
template <class TPanel = app::ThemedSvgPanel>
TPanel* createPanel(std::string lightSvgPath, std::string darkSvgPath) {
	TPanel* panel = new TPanel;
	panel->setBackground(window::Svg::load(lightSvgPath), window::Svg::load(darkSvgPath));
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


/** Creates a param with a light and calls setFirstLightId() on it.
Requires ParamWidget to have a `light` member.
*/
template <class TParamWidget>
TParamWidget* createLightParam(math::Vec pos, engine::Module* module, int paramId, int firstLightId) {
	TParamWidget* o = createParam<TParamWidget>(pos, module, paramId);
	o->getLight()->module = module;
	o->getLight()->firstLightId = firstLightId;
	return o;
}


template <class TParamWidget>
TParamWidget* createLightParamCentered(math::Vec pos, engine::Module* module, int paramId, int firstLightId) {
	TParamWidget* o = createLightParam<TParamWidget>(pos, module, paramId, firstLightId);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
	return o;
}


template <class TMenu = ui::Menu>
TMenu* createMenu() {
	TMenu* menu = new TMenu;
	menu->box.pos = APP->scene->mousePos;

	ui::MenuOverlay* menuOverlay = new ui::MenuOverlay;
	menuOverlay->addChild(menu);

	APP->scene->addChild(menuOverlay);
	return menu;
}


template <class TMenuLabel = ui::MenuLabel>
TMenuLabel* createMenuLabel(std::string text) {
	TMenuLabel* label = new TMenuLabel;
	label->text = text;
	return label;
}


template <class TMenuItem = ui::MenuItem>
TMenuItem* createMenuItem(std::string text, std::string rightText = "") {
	TMenuItem* item = new TMenuItem;
	item->text = text;
	item->rightText = rightText;
	return item;
}


/** Creates a MenuItem with an action that calls a lambda function.
Example:

	menu->addChild(createMenuItem("Load sample", "kick.wav",
		[=]() {
			module->loadSample();
		}
	));
*/
template <class TMenuItem = ui::MenuItem>
TMenuItem* createMenuItem(std::string text, std::string rightText, std::function<void()> action, bool disabled = false, bool alwaysConsume = false) {
	struct Item : TMenuItem {
		std::function<void()> action;
		bool alwaysConsume;

		void onAction(const event::Action& e) override {
			action();
			if (alwaysConsume)
				e.consume(this);
		}
	};

	Item* item = createMenuItem<Item>(text, rightText);
	item->action = action;
	item->disabled = disabled;
	item->alwaysConsume = alwaysConsume;
	return item;
}


/** Creates a MenuItem with a check mark set by a lambda function.
Example:

	menu->addChild(createCheckMenuItem("Loop", "",
		[=]() {
			return module->isLoop();
		},
		[=]() {
			module->toggleLoop();
		}
	));
*/
template <class TMenuItem = ui::MenuItem>
ui::MenuItem* createCheckMenuItem(std::string text, std::string rightText, std::function<bool()> checked, std::function<void()> action, bool disabled = false, bool alwaysConsume = false) {
	struct Item : TMenuItem {
		std::string rightTextPrefix;
		std::function<bool()> checked;
		std::function<void()> action;
		bool alwaysConsume;

		void step() override {
			this->rightText = rightTextPrefix;
			if (checked()) {
				if (!rightTextPrefix.empty())
					this->rightText += "  ";
				this->rightText += CHECKMARK_STRING;
			}
			TMenuItem::step();
		}
		void onAction(const event::Action& e) override {
			action();
			if (alwaysConsume)
				e.consume(this);
		}
	};

	Item* item = createMenuItem<Item>(text);
	item->rightTextPrefix = rightText;
	item->checked = checked;
	item->action = action;
	item->disabled = disabled;
	item->alwaysConsume = alwaysConsume;
	return item;
}


/** Creates a MenuItem that controls a boolean value with a check mark.
Example:

	menu->addChild(createBoolMenuItem("Loop", "",
		[=]() {
			return module->isLoop();
		},
		[=](bool loop) {
			module->setLoop(loop);
		}
	));
*/
template <class TMenuItem = ui::MenuItem>
ui::MenuItem* createBoolMenuItem(std::string text, std::string rightText, std::function<bool()> getter, std::function<void(bool state)> setter, bool disabled = false, bool alwaysConsume = false) {
	struct Item : TMenuItem {
		std::string rightTextPrefix;
		std::function<bool()> getter;
		std::function<void(size_t)> setter;
		bool alwaysConsume;

		void step() override {
			this->rightText = rightTextPrefix;
			if (getter()) {
				if (!rightTextPrefix.empty())
					this->rightText += "  ";
				this->rightText += CHECKMARK_STRING;
			}
			TMenuItem::step();
		}
		void onAction(const event::Action& e) override {
			setter(!getter());
			if (alwaysConsume)
				e.consume(this);
		}
	};

	Item* item = createMenuItem<Item>(text);
	item->rightTextPrefix = rightText;
	item->getter = getter;
	item->setter = setter;
	item->disabled = disabled;
	item->alwaysConsume = alwaysConsume;
	return item;
}


/** Easy wrapper for createBoolMenuItem() to modify a bool pointer.
Example:

	menu->addChild(createBoolPtrMenuItem("Loop", "", &module->loop));
*/
template <typename T>
ui::MenuItem* createBoolPtrMenuItem(std::string text, std::string rightText, T* ptr) {
	return createBoolMenuItem(text, rightText,
		[=]() {
			return ptr ? *ptr : false;
		},
		[=](T val) {
			if (ptr)
				*ptr = val;
		}
	);
}


/** Creates a MenuItem that opens a submenu.
Example:

	menu->addChild(createSubmenuItem("Edit", "",
		[=](Menu* menu) {
			menu->addChild(createMenuItem("Copy", "", [=]() {copy();}));
			menu->addChild(createMenuItem("Paste", "", [=]() {paste();}));
		}
	));
*/
template <class TMenuItem = ui::MenuItem>
ui::MenuItem* createSubmenuItem(std::string text, std::string rightText, std::function<void(ui::Menu* menu)> createMenu, bool disabled = false) {
	struct Item : TMenuItem {
		std::function<void(ui::Menu* menu)> createMenu;

		ui::Menu* createChildMenu() override {
			ui::Menu* menu = new ui::Menu;
			createMenu(menu);
			return menu;
		}
	};

	Item* item = createMenuItem<Item>(text, rightText + (rightText.empty() ? "" : "  ") + RIGHT_ARROW);
	item->createMenu = createMenu;
	item->disabled = disabled;
	return item;
}


/** Creates a MenuItem that when hovered, opens a submenu with several MenuItems indexed by an integer.
Example:

	menu->addChild(createIndexSubmenuItem("Mode",
		{"Hi-fi", "Mid-fi", "Lo-fi"},
		[=]() {
			return module->getMode();
		},
		[=](int mode) {
			module->setMode(mode);
		}
	));
*/
template <class TMenuItem = ui::MenuItem>
ui::MenuItem* createIndexSubmenuItem(std::string text, std::vector<std::string> labels, std::function<size_t()> getter, std::function<void(size_t val)> setter, bool disabled = false, bool alwaysConsume = false) {
	struct IndexItem : ui::MenuItem {
		std::function<size_t()> getter;
		std::function<void(size_t)> setter;
		size_t index;
		bool alwaysConsume;

		void step() override {
			size_t currIndex = getter();
			this->rightText = CHECKMARK(currIndex == index);
			MenuItem::step();
		}
		void onAction(const event::Action& e) override {
			setter(index);
			if (alwaysConsume)
				e.consume(this);
		}
	};

	struct Item : TMenuItem {
		std::function<size_t()> getter;
		std::function<void(size_t)> setter;
		std::vector<std::string> labels;
		bool alwaysConsume;

		void step() override {
			size_t currIndex = getter();
			std::string label = (currIndex < labels.size()) ? labels[currIndex] : "";
			this->rightText = label + "  " + RIGHT_ARROW;
			TMenuItem::step();
		}
		ui::Menu* createChildMenu() override {
			ui::Menu* menu = new ui::Menu;
			for (size_t i = 0; i < labels.size(); i++) {
				IndexItem* item = createMenuItem<IndexItem>(labels[i]);
				item->getter = getter;
				item->setter = setter;
				item->index = i;
				item->alwaysConsume = alwaysConsume;
				menu->addChild(item);
			}
			return menu;
		}
	};

	Item* item = createMenuItem<Item>(text);
	item->getter = getter;
	item->setter = setter;
	item->labels = labels;
	item->disabled = disabled;
	item->alwaysConsume = alwaysConsume;
	return item;
}


/** Easy wrapper for createIndexSubmenuItem() that controls an integer index at a pointer address.
Example:

	menu->addChild(createIndexPtrSubmenuItem("Mode",
		{"Hi-fi", "Mid-fi", "Lo-fi"},
		&module->mode
	));
*/
template <typename T>
ui::MenuItem* createIndexPtrSubmenuItem(std::string text, std::vector<std::string> labels, T* ptr) {
	return createIndexSubmenuItem(text, labels,
		[=]() {
			return ptr ? *ptr : 0;
		},
		[=](size_t index) {
			if (ptr)
				*ptr = T(index);
		}
	);
}


} // namespace rack
