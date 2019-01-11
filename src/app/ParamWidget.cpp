#include "app/ParamWidget.hpp"
#include "ui/MenuOverlay.hpp"
#include "ui/TextField.hpp"
#include "app/Scene.hpp"
#include "app/ParamQuantity.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "random.hpp"
#include "history.hpp"


namespace rack {


struct ParamField : TextField {
	ParamWidget *paramWidget;

	void step() override {
		// Keep selected
		app()->event->setSelected(this);
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

	if (tooltip) {
		if (paramQuantity) {
			// Quantity string
			tooltip->text = paramQuantity->getString();
			// Param description
			std::string description = paramQuantity->getParam()->description;
			if (!description.empty())
				tooltip->text += "\n" + description;
		}
		// Position at bottom-right of parameter
		tooltip->box.pos = getAbsoluteOffset(box.size).round();
	}

	OpaqueWidget::step();
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ) {
		if (paramQuantity)
			paramQuantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::onButton(const event::Button &e) {
	// Right click to reset
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && !(e.mods & WINDOW_MOD) && !(e.mods & GLFW_MOD_SHIFT)) {
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
		}
		// Here's another way of doing it, but either works.
		// paramQuantity->getParam()->reset();
		e.consume(this);
	}

	// Shift-click to open value entry
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && !(e.mods & WINDOW_MOD) && (e.mods & GLFW_MOD_SHIFT)) {
		// Create ParamField
		MenuOverlay *overlay = new MenuOverlay;
		app()->scene->addChild(overlay);

		ParamField *paramField = new ParamField;
		paramField->box.size.x = 100;
		paramField->box.pos = getAbsoluteOffset(box.size).round();
		paramField->setParamWidget(this);
		overlay->addChild(paramField);
		e.consume(this);
	}

	if (!e.getConsumed())
		OpaqueWidget::onButton(e);
}

void ParamWidget::onEnter(const event::Enter &e) {
	if (settings::paramTooltip && !tooltip) {
		tooltip = new Tooltip;
		app()->scene->addChild(tooltip);
	}
}

void ParamWidget::onLeave(const event::Leave &e) {
	if (tooltip) {
		app()->scene->removeChild(tooltip);
		delete tooltip;
		tooltip = NULL;
	}
}


} // namespace rack
