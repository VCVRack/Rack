#include "app/ParamWidget.hpp"
#include "ui/MenuOverlay.hpp"
#include "ui/TextField.hpp"
#include "app/Scene.hpp"
#include "app/ParamQuantity.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "random.hpp"
#include "history.hpp"
#include "helpers.hpp"


namespace rack {


struct ParamField : TextField {
	ParamWidget *paramWidget;

	void step() override {
		// Keep selected
		app()->event->setSelected(this);
		TextField::step();
	}

	void setParamWidget(ParamWidget *paramWidget) {
		this->paramWidget = paramWidget;
		if (paramWidget->paramQuantity)
			text = paramWidget->paramQuantity->getDisplayValueString();
		selectAll();
	}

	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			float oldValue = paramWidget->paramQuantity->getValue();
			if (paramWidget->paramQuantity)
				paramWidget->paramQuantity->setDisplayValueString(text);
			float newValue = paramWidget->paramQuantity->getValue();

			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange *h = new history::ParamChange;
				h->moduleId = paramWidget->paramQuantity->module->id;
				h->paramId = paramWidget->paramQuantity->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				app()->history->push(h);
			}

			MenuOverlay *overlay = getAncestorOfType<MenuOverlay>();
			overlay->requestedDelete = true;
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
			MenuOverlay *overlay = getAncestorOfType<MenuOverlay>();
			overlay->requestedDelete = true;
			e.consume(this);
		}

		if (!e.getConsumed())
			TextField::onSelectKey(e);
	}
};


struct ParamTooltip : Tooltip {
	ParamWidget *paramWidget;

	void step() override {
		if (paramWidget->paramQuantity) {
			// Quantity string
			text = paramWidget->paramQuantity->getString();
			// Param description
			std::string description = paramWidget->paramQuantity->getParam()->description;
			if (!description.empty())
				text += "\n" + description;
		}
		// Position at bottom-right of parameter
		box.pos = paramWidget->getAbsoluteOffset(box.size).round();
	}
};


struct ParamResetItem : MenuItem {
	ParamWidget *paramWidget;
	ParamResetItem() {
		text = "Initialize";
		rightText = WINDOW_MOD_ALT_NAME "+click";
	}
	void onAction(const event::Action &e) override {
		paramWidget->resetAction();
	}
};


struct ParamFieldItem : MenuItem {
	ParamWidget *paramWidget;
	ParamFieldItem() {
		text = "Enter value";
		rightText = WINDOW_MOD_SHIFT_NAME "+click";
	}
	void onAction(const event::Action &e) override {
		paramWidget->createParamField();
	}
};


ParamWidget::~ParamWidget() {
	if (paramQuantity)
		delete paramQuantity;
}

void ParamWidget::step() {
	if (paramQuantity) {
		float value = paramQuantity->getValue();
		// Trigger change event when paramQuantity value changes
		if (value != dirtyValue) {
			dirtyValue = value;
			event::Change eChange;
			onChange(eChange);
		}
	}

	OpaqueWidget::step();
}

void ParamWidget::draw(NVGcontext *vg) {
	Widget::draw(vg);

	// if (paramQuantity) {
	// 	nvgBeginPath(vg);
	// 	nvgRect(vg,
	// 		box.size.x - 12, box.size.y - 12,
	// 		12, 12);
	// 	nvgFillColor(vg, nvgRGBAf(1, 0, 1, 0.9));
	// 	nvgFill(vg);

	// 	std::string mapText = string::f("%d", paramQuantity->paramId);
	// 	bndLabel(vg, box.size.x - 17.0, box.size.y - 16.0, INFINITY, INFINITY, -1, mapText.c_str());
	// }
}

void ParamWidget::onButton(const event::Button &e) {
	// Right click to open context menu
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & WINDOW_MOD_MASK) == 0) {
		createContextMenu();
		e.consume(this);
	}

	// Alt-click to reset
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & WINDOW_MOD_MASK) == GLFW_MOD_ALT) {
		resetAction();
		e.consume(this);
	}

	// Shift-click to open value entry
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & WINDOW_MOD_MASK) == GLFW_MOD_SHIFT) {
		createParamField();
		e.consume(this);
	}

	if (!e.getConsumed())
		OpaqueWidget::onButton(e);
}

void ParamWidget::onEnter(const event::Enter &e) {
	if (settings::paramTooltip && !tooltip) {
		ParamTooltip *paramTooltip = new ParamTooltip;
		paramTooltip->paramWidget = this;
		app()->scene->addChild(paramTooltip);
		tooltip = paramTooltip;
	}
}

void ParamWidget::onLeave(const event::Leave &e) {
	if (tooltip) {
		app()->scene->removeChild(tooltip);
		delete tooltip;
		tooltip = NULL;
	}
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ) {
		if (paramQuantity)
			paramQuantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::createParamField() {
	// Create ParamField
	MenuOverlay *overlay = new MenuOverlay;
	app()->scene->addChild(overlay);

	ParamField *paramField = new ParamField;
	paramField->box.size.x = 100;
	paramField->box.pos = getAbsoluteOffset(box.size).round();
	paramField->setParamWidget(this);
	overlay->addChild(paramField);
}

void ParamWidget::createContextMenu() {
	Menu *menu = createMenu();

	ParamResetItem *resetItem = new ParamResetItem;
	resetItem->paramWidget = this;
	menu->addChild(resetItem);

	ParamFieldItem *fieldItem = new ParamFieldItem;
	fieldItem->paramWidget = this;
	menu->addChild(fieldItem);
}

void ParamWidget::resetAction() {
	if (paramQuantity && paramQuantity->isBounded()) {
		float oldValue = paramQuantity->getValue();
		paramQuantity->reset();
		float newValue = paramQuantity->getValue();

		if (oldValue != newValue) {
			// Push ParamChange history action
			history::ParamChange *h = new history::ParamChange;
			h->moduleId = paramQuantity->module->id;
			h->paramId = paramQuantity->paramId;
			h->oldValue = oldValue;
			h->newValue = newValue;
			app()->history->push(h);
		}

		// Here's another way of doing it, but either works.
		// paramQuantity->getParam()->reset();
	}
}


} // namespace rack
