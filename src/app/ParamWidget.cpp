#include "app/ParamWidget.hpp"
#include "random.hpp"


namespace rack {


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

	OpaqueWidget::step();
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ) {
		if (quantity)
			quantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::onButton(event::Button &e) {
	// Right click to reset
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (quantity)
			quantity->reset();
		// Here's another way of doing it, but either works.
		// dynamic_cast<ParamQuantity*>(quantity)->getParam()->reset();
	}

	OpaqueWidget::onButton(e);
}

void ParamWidget::onDragMove(event::DragMove &e) {
	if (quantity) {
		DEBUG("%s", quantity->getString().c_str());
	}
}


} // namespace rack
