#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;


Knob::Knob() {
	smooth = true;
}

void Knob::onDragStart(EventDragStart &e) {
	windowCursorLock();
	dragValue = value;
	randomizable = false;
}

void Knob::onDragMove(EventDragMove &e) {
	float range;
	if (isfinite(minValue) && isfinite(maxValue)) {
		range = maxValue - minValue;
	}
	else {
		// Continuous encoders scale as if their limits are +/-1
		range = 1.f - (-1.f);
	}
	float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed * range;

	// Drag slower if Mod is held
	if (windowIsModPressed())
		delta /= 16.f;
	dragValue += delta;
	dragValue = clamp2(dragValue, minValue, maxValue);
	if (snap)
		setValue(roundf(dragValue));
	else
		setValue(dragValue);
}

void Knob::onDragEnd(EventDragEnd &e) {
	windowCursorUnlock();
	randomizable = true;
}


} // namespace rack
