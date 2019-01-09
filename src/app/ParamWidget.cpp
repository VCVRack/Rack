#include "app/ParamWidget.hpp"
#include "ui/MenuOverlay.hpp"
#include "ui/TextField.hpp"
#include "app/Scene.hpp"
#include "app/ParamQuantity.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "random.hpp"


namespace rack {


struct ParamField : TextField {
	ParamWidget *paramWidget;

	void step() override {
		// Keep selected
		app()->event->setSelected(this);
	}

	void setParamWidget(ParamWidget *paramWidget) {
		this->paramWidget = paramWidget;
		if (paramWidget->quantity)
			text = paramWidget->quantity->getDisplayValueString();
		selectAll();
	}

	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			if (paramWidget->quantity)
				paramWidget->quantity->setDisplayValueString(text);

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
	if (quantity)
		delete quantity;
}

void ParamWidget::step() {
	if (quantity) {
		float value = quantity->getValue();
		// Trigger change event when quantity value changes
		if (value != dirtyValue) {
			dirtyValue = value;
			event::Change eChange;
			onChange(eChange);
		}
	}

	if (tooltip) {
		// Quantity string
		if (quantity) {
			tooltip->text = quantity->getString();
		}
		// Param description
		ParamQuantity *paramQuantity = dynamic_cast<ParamQuantity*>(quantity);
		if (paramQuantity) {
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
		if (quantity)
			quantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::onButton(const event::Button &e) {
	// Right click to reset
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && !(e.mods & WINDOW_MOD) && !(e.mods & GLFW_MOD_SHIFT)) {
		if (quantity)
			quantity->reset();
		// Here's another way of doing it, but either works.
		// dynamic_cast<ParamQuantity*>(quantity)->getParam()->reset();
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
