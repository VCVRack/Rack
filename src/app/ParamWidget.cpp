#include "app/ParamWidget.hpp"
#include "app/Scene.hpp"
#include "context.hpp"
#include "settings.hpp"
#include "random.hpp"


namespace rack {


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
		if (quantity)
			tooltip->text = quantity->getString();
		tooltip->box.pos = getAbsoluteOffset(box.size);
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
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (quantity)
			quantity->reset();
		// Here's another way of doing it, but either works.
		// dynamic_cast<ParamQuantity*>(quantity)->getParam()->reset();
	}

	OpaqueWidget::onButton(e);
}

void ParamWidget::onEnter(const event::Enter &e) {
	if (settings::paramTooltip && !tooltip) {
		tooltip = new Tooltip;
		context()->scene->addChild(tooltip);
	}
}

void ParamWidget::onLeave(const event::Leave &e) {
	if (tooltip) {
		context()->scene->removeChild(tooltip);
		delete tooltip;
		tooltip = NULL;
	}
}


} // namespace rack
