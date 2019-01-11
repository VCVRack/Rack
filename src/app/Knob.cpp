#include "app/Knob.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;


void Knob::onButton(const event::Button &e) {
	float r = box.size.x / 2;
	math::Vec c = box.size.div(2);
	float dist = e.pos.minus(c).norm();
	if (dist <= r) {
		ParamWidget::onButton(e);
	}
}

void Knob::onDragStart(const event::DragStart &e) {
	if (paramQuantity)
		oldValue = paramQuantity->getValue();

	app()->window->cursorLock();
}

void Knob::onDragEnd(const event::DragEnd &e) {
	app()->window->cursorUnlock();

	if (paramQuantity) {
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

void Knob::onDragMove(const event::DragMove &e) {
	if (paramQuantity) {
		float range;
		if (paramQuantity->isBounded()) {
			range = paramQuantity->getRange();
		}
		else {
			// Continuous encoders scale as if their limits are +/-1
			range = 2.f;
		}
		float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed * range;

		// Drag slower if Mod is held
		if (app()->window->isModPressed())
			delta /= 16.f;
		float oldValue = paramQuantity->getSmoothValue();
		paramQuantity->setSmoothValue(oldValue + delta);
	}

	ParamWidget::onDragMove(e);
}


} // namespace rack
