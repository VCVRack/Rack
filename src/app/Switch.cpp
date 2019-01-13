#include "app/Switch.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {


void Switch::onDragStart(const event::DragStart &e) {
	// Cycle through values
	// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
	if (paramQuantity) {
		float oldValue = paramQuantity->getValue();
		if (momentary) {
			// Set to maximum value
			paramQuantity->setMax();
		}
		else {
			// Increment value by 1, or reset back to minimum
			if (paramQuantity->isMax()) {
				paramQuantity->setMin();
			}
			else {
				paramQuantity->setValue(std::floor(paramQuantity->getValue() + 1));
			}
		}

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
}

void Switch::onDragEnd(const event::DragEnd &e) {
	if (paramQuantity) {
		if (momentary) {
			// Set to minimum value
			paramQuantity->setMin();
		}
	}
}


} // namespace rack
