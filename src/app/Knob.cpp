#include "app/Knob.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;


void Knob::onHover(const event::Hover &e) {
	math::Vec c = box.size.div(2);
	float dist = e.pos.minus(c).norm();
	if (dist <= c.x) {
		ParamWidget::onHover(e);
	}
}

void Knob::onButton(const event::Button &e) {
	math::Vec c = box.size.div(2);
	float dist = e.pos.minus(c).norm();
	if (dist <= c.x) {
		ParamWidget::onButton(e);
	}
}

void Knob::onDragStart(const event::DragStart &e) {
	if (paramQuantity) {
		oldValue = paramQuantity->getSmoothValue();
		if (snap) {
			snapValue = paramQuantity->getValue();
		}
	}

	app()->window->cursorLock();
}

void Knob::onDragEnd(const event::DragEnd &e) {
	app()->window->cursorUnlock();

	if (paramQuantity) {
		float newValue = paramQuantity->getSmoothValue();
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

		// Drag slower if shift is held
		if ((app()->window->getMods() & WINDOW_MOD_MASK) == GLFW_MOD_SHIFT)
			delta /= 16.f;

		if (snap) {
			snapValue += delta;
			snapValue = math::clamp(snapValue, paramQuantity->getMinValue(), paramQuantity->getMaxValue());
			paramQuantity->setValue(std::round(snapValue));
		}
		else if (smooth) {
			paramQuantity->setSmoothValue(paramQuantity->getSmoothValue() + delta);
		}
		else {
			paramQuantity->setValue(paramQuantity->getValue() + delta);
		}
	}

	ParamWidget::onDragMove(e);
}


} // namespace rack
