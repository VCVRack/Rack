#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {

#define KNOB_SENSITIVITY 0.0015


void Knob::onDragStart(EventDragStart &e) {
	windowCursorLock();
	dragValue = value;
	randomizable = false;
}

void Knob::onDragMove(EventDragMove &e) {
	float range = maxValue - minValue;
	float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed;
	if (std::isfinite(range))
		delta *= range;

	// Drag slower if Mod is held
	if (windowIsModPressed())
		delta /= 16.0;
	dragValue += delta;
	if (snap)
		setValue(roundf(dragValue));
	else
		setValue(dragValue);
}

void Knob::onDragEnd(EventDragEnd &e) {
	windowCursorUnlock();
	randomizable = true;
}

void Knob::onChange(EventChange &e) {
	if (!module)
		return;

	if (snap)
		engineSetParam(module, paramId, value);
	else
		engineSetParamSmooth(module, paramId, value);
}


} // namespace rack
