#include "app/ParamWidget.hpp"
#include "random.hpp"


namespace rack {


void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ) {
		if (quantity)
			quantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::reset() {
	if (quantity) {
		// Infinite params should not be reset
		if (quantity->isBounded())
			quantity->reset();
	}
}

void ParamWidget::randomize() {
	if (quantity) {
		// Infinite params should not be randomized
		if (quantity->isBounded()) {
			quantity->setScaledValue(random::uniform());
		}
	}
}

void ParamWidget::onButton(event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.target == this) {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (quantity)
				quantity->reset();
		}
	}
}


} // namespace rack
