#include <app/Knob.hpp>
#include <context.hpp>
#include <app/Scene.hpp>
#include <random.hpp>
#include <history.hpp>
#include <settings.hpp>


namespace rack {
namespace app {


struct Knob::Internal {
	/** Value of the knob before dragging. */
	float oldValue = 0.f;
	/** Fractional value between the param's value and the dragged knob position.
	Using a "snapValue" variable and rounding is insufficient because the mouse needs to reach 1.0, not 0.5 to obtain the first increment.
	*/
	float snapDelta = 0.f;

	/** Speed multiplier in speed knob mode */
	float linearScale = 1.f;
	/** The mouse has once escaped from the knob while dragging. */
	bool rotaryDragEnabled = false;
	float dragAngle = NAN;
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
	// Only call super if mouse position is in the circle
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

	settings::KnobMode km = settings::knobMode;
	if (km == settings::KNOB_MODE_LINEAR || km == settings::KNOB_MODE_SCALED_LINEAR) {
		APP->window->cursorLock();
	}
	// Only changed for KNOB_MODE_LINEAR_*.
	internal->linearScale = 1.f;
	// Only used for KNOB_MODE_ROTARY_*.
	internal->rotaryDragEnabled = false;
	internal->dragAngle = NAN;

	ParamWidget::onDragStart(e);
}

void Knob::onDragEnd(const event::DragEnd& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	settings::KnobMode km = settings::knobMode;
	if (km == settings::KNOB_MODE_LINEAR || km == settings::KNOB_MODE_SCALED_LINEAR) {
		APP->window->cursorUnlock();
	}

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
		// Reset snap delta
		internal->snapDelta = 0.f;
	}

	ParamWidget::onDragEnd(e);
}

static float getModSpeed() {
	int mods = APP->window->getMods();
	if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL)
		return 1 / 16.f;
	else if ((mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
		return 4.f;
	else if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT))
		return 1 / 256.f;
	else
		return 1.f;
}

void Knob::onDragMove(const event::DragMove& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	settings::KnobMode km = settings::knobMode;
	bool linearMode = (km == settings::KNOB_MODE_LINEAR || km == settings::KNOB_MODE_SCALED_LINEAR) || forceLinear;

	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		float value = smooth ? pq->getSmoothValue() : pq->getValue();

		// Ratio between parameter value scale / (angle range / 2*pi)
		float rangeRatio;
		if (pq->isBounded()) {
			rangeRatio = pq->getRange();
			rangeRatio /= (maxAngle - minAngle) / float(2 * M_PI);
		}
		else {
			rangeRatio = 1.f;
		}

		if (linearMode) {
			float delta = (horizontal ? e.mouseDelta.x : -e.mouseDelta.y);
			delta *= settings::knobLinearSensitivity;
			delta *= getModSpeed();
			delta *= rangeRatio;

			// Scale delta if in scaled linear knob mode
			if (km == settings::KNOB_MODE_SCALED_LINEAR) {
				float deltaY = (horizontal ? -e.mouseDelta.y : -e.mouseDelta.x);
				const float pixelTau = 200.f;
				internal->linearScale *= std::pow(2.f, -deltaY / pixelTau);
				delta *= internal->linearScale;
			}

			// Handle value snapping
			if (pq->snapEnabled) {
				// Replace delta with an accumulated delta since the last integer knob.
				internal->snapDelta += delta;
				delta = std::trunc(internal->snapDelta);
				internal->snapDelta -= delta;
			}

			value += delta;
		}
		else if (internal->rotaryDragEnabled) {
			math::Vec origin = getAbsoluteOffset(box.size.div(2));
			math::Vec deltaPos = APP->scene->mousePos.minus(origin);
			float angle = deltaPos.arg() + float(M_PI) / 2;

			bool absoluteRotaryMode = (km == settings::KNOB_MODE_ROTARY_ABSOLUTE) && pq->isBounded();
			if (absoluteRotaryMode) {
				// Find angle closest to midpoint of angle range, mod 2*pi
				float midAngle = (minAngle + maxAngle) / 2;
				angle = math::eucMod(angle - midAngle + float(M_PI), float(2 * M_PI)) + midAngle - float(M_PI);
				value = math::rescale(angle, minAngle, maxAngle, pq->getMinValue(), pq->getMaxValue());
			}
			else {
				if (!std::isfinite(internal->dragAngle)) {
					// Set the starting angle
					internal->dragAngle = angle;
				}

				// Find angle closest to last angle, mod 2*pi
				float deltaAngle = math::eucMod(angle - internal->dragAngle + float(M_PI), float(2 * M_PI)) - float(M_PI);
				internal->dragAngle = angle;
				float delta = deltaAngle / float(2 * M_PI) * rangeRatio;
				delta *= getModSpeed();

				// Handle value snapping
				if (pq->snapEnabled) {
					// Replace delta with an accumulated delta since the last integer knob.
					internal->snapDelta += delta;
					delta = std::trunc(internal->snapDelta);
					internal->snapDelta -= delta;
				}

				value += delta;
			}
		}

		// Set value
		if (smooth)
			pq->setSmoothValue(value);
		else
			pq->setValue(value);
	}

	ParamWidget::onDragMove(e);
}

void Knob::onDragLeave(const event::DragLeave& e) {
	if (e.origin == this) {
		internal->rotaryDragEnabled = true;
	}

	ParamWidget::onDragLeave(e);
}


void Knob::onHoverScroll(const event::HoverScroll& e) {
	ParamWidget::onHoverScroll(e);

	if (settings::knobScroll) {
		engine::ParamQuantity* pq = getParamQuantity();
		if (pq) {
			float value = smooth ? pq->getSmoothValue() : pq->getValue();

			float rangeRatio;
			if (pq->isBounded()) {
				rangeRatio = pq->getRange();
			}
			else {
				rangeRatio = 1.f;
			}

			float delta = e.scrollDelta.y;
			delta *= settings::knobScrollSensitivity;
			delta *= getModSpeed();
			delta *= rangeRatio;

			// Handle value snapping
			if (pq->snapEnabled) {
				// Replace delta with an accumulated delta since the last integer knob.
				internal->snapDelta += delta;
				delta = std::trunc(internal->snapDelta);
				internal->snapDelta -= delta;
			}

			value += delta;
			if (smooth)
				pq->setSmoothValue(value);
			else
				pq->setValue(value);

			e.consume(this);
		}
	}
}


} // namespace app
} // namespace rack
