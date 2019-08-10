#include <app/Switch.hpp>
#include <app.hpp>
#include <app/Scene.hpp>
#include <random.hpp>
#include <history.hpp>


namespace rack {
namespace app {


void Switch::step() {
	if (momentaryPressed) {
		momentaryPressed = false;
		// Wait another frame.
	}
	else if (momentaryReleased) {
		momentaryReleased = false;
		if (paramQuantity) {
			// Set to minimum value
			paramQuantity->setMin();
		}
	}
	ParamWidget::step();
}

void Switch::onDoubleClick(const event::DoubleClick& e) {
	// Don't reset parameter on double-click
}

void Switch::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (momentary) {
		if (paramQuantity) {
			// Set to maximum value
			paramQuantity->setMax();
			momentaryPressed = true;
		}
	}
	else {
		if (paramQuantity) {
			float oldValue = paramQuantity->getValue();
			if (paramQuantity->isMax()) {
				// Reset value back to minimum
				paramQuantity->setMin();
			}
			else {
				// Increment value by 1
				paramQuantity->setValue(std::round(paramQuantity->getValue()) + 1.f);
			}

			float newValue = paramQuantity->getValue();
			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->name = "move switch";
				h->moduleId = paramQuantity->module->id;
				h->paramId = paramQuantity->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}
		}
	}
}

void Switch::onDragEnd(const event::DragEnd& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (momentary) {
		momentaryReleased = true;
	}
}

void Switch::reset() {
	if (paramQuantity && !momentary) {
		paramQuantity->reset();
	}
}

void Switch::randomize() {
	if (paramQuantity && !momentary) {
		float value = paramQuantity->getMinValue() + std::floor(random::uniform() * (paramQuantity->getRange() + 1));
		paramQuantity->setValue(value);
	}
}


} // namespace app
} // namespace rack
