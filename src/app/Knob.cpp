#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {

#define KNOB_SENSITIVITY 0.0015


void Knob::onDragStart(EventDragStart &e) {
	guiCursorLock();
	dragValue = value;
	randomizable = false;
}

void Knob::onDragMove(EventDragMove &e) {
	// Drag slower if Mod
	float range = maxValue - minValue;
	float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed;
	if (std::isfinite(range))
		delta *= range;

	if (guiIsModPressed())
		delta /= 16.0;
	dragValue += delta;
	if (snap)
		setValue(roundf(dragValue));
	else
		setValue(dragValue);
}

void Knob::onDragEnd(EventDragEnd &e) {
	guiCursorUnlock();
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
