#include <app/Knob.hpp>
#include <app.hpp>
#include <app/Scene.hpp>
#include <random.hpp>
#include <history.hpp>


namespace rack {
namespace app {


static const float KNOB_SENSITIVITY = 0.0015f;


struct Knob::Internal {
	/** Value of the knob before dragging. */
	float oldValue = 0.f;
	/** Fractional value between the param's value and the dragged knob position. */
	float snapDelta = 0.f;
};


Knob::Knob() {
	internal = new Internal;
}

Knob::~Knob() {
	delete internal;
}

void Knob::initParamQuantity() {
	ParamWidget::initParamQuantity();
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		if (snap)
			pq->snapEnabled = true;
	}
}

void Knob::onHover(const event::Hover& e) {
	math::Vec c = box.size.div(2);
	float dist = e.pos.minus(c).norm();
	if (dist <= c.x) {
		ParamWidget::onHover(e);
	}
}

void Knob::onButton(const event::Button& e) {
	math::Vec c = box.size.div(2);
	float dist = e.pos.minus(c).norm();
	if (dist <= c.x) {
		ParamWidget::onButton(e);
	}
}

void Knob::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		internal->oldValue = pq->getSmoothValue();
		internal->snapDelta = 0.f;
	}

	APP->window->cursorLock();
}

void Knob::onDragEnd(const event::DragEnd& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	APP->window->cursorUnlock();

	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		float newValue = pq->getSmoothValue();
		if (internal->oldValue != newValue) {
			// Push ParamChange history action
			history::ParamChange* h = new history::ParamChange;
			h->name = "move knob";
			h->moduleId = module->id;
			h->paramId = paramId;
			h->oldValue = internal->oldValue;
			h->newValue = newValue;
			APP->history->push(h);
		}
	}
}

void Knob::onDragMove(const event::DragMove& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		float range;
		if (pq->isBounded()) {
			range = pq->getRange();
		}
		else {
			// Continuous encoders scale as if their limits are +/-1
			range = 2.f;
		}
		float delta = (horizontal ? e.mouseDelta.x : -e.mouseDelta.y);
		delta *= KNOB_SENSITIVITY;
		delta *= speed;
		delta *= range;

		// Drag slower if Mod is held
		int mods = APP->window->getMods();
		if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			delta /= 16.f;
		}
		// Drag even slower if Mod-Shift is held
		if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			delta /= 256.f;
		}

		if (pq->snapEnabled) {
			// Replace delta with an accumulated delta since the last integer knob.
			internal->snapDelta += delta;
			delta = std::trunc(internal->snapDelta);
			internal->snapDelta -= delta;
		}

		// Set value
		if (smooth) {
			pq->setSmoothValue(pq->getSmoothValue() + delta);
		}
		else {
			pq->setValue(pq->getValue() + delta);
		}
	}

	ParamWidget::onDragMove(e);
}


} // namespace app
} // namespace rack
