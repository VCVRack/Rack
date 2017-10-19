#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {

#define KNOB_SENSITIVITY 0.0015


void Knob::onDragStart() {
	guiCursorLock();
	dragValue = value;
	randomizable = false;
}

void Knob::onDragMove(Vec mouseRel) {
	// Drag slower if Mod
	if (guiIsModPressed())
		mouseRel = mouseRel.mult(1/16.0);
	dragValue += KNOB_SENSITIVITY * (maxValue - minValue) * -mouseRel.y;
	if (snap)
		setValue(roundf(dragValue));
	else
		setValue(dragValue);
}

void Knob::onDragEnd() {
	guiCursorUnlock();
	randomizable = true;
}

void Knob::onChange() {
	if (!module)
		return;

	if (snap)
		engineSetParam(module, paramId, value);
	else
		engineSetParamSmooth(module, paramId, value);
}


} // namespace rack
