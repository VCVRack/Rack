#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {

#define KNOB_SENSITIVITY 0.0015


void Knob::onDragStart() {
	guiCursorLock();
}

void Knob::onDragMove(Vec mouseRel) {
	// Drag slower if Mod
	if (guiIsModPressed())
		mouseRel = mouseRel.mult(0.1);
	setValue(value - KNOB_SENSITIVITY * (maxValue - minValue) * mouseRel.y);
}

void Knob::onDragEnd() {
	guiCursorUnlock();
}

void Knob::onChange() {
	if (!module)
		return;

	engineSetParamSmooth(module, paramId, value);
}


} // namespace rack
