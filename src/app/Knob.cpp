#include "app.hpp"
#include "gui.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {

#define KNOB_SENSITIVITY 0.001


void Knob::onDragStart() {
	guiCursorLock();
}

void Knob::onDragMove(Vec mouseRel) {
	// Drag slower if Ctrl is held
	if (guiIsKeyPressed(GLFW_KEY_LEFT_CONTROL) || guiIsKeyPressed(GLFW_KEY_RIGHT_CONTROL))
		mouseRel = mouseRel.mult(0.1);
	setValue(value - KNOB_SENSITIVITY * (maxValue - minValue) * mouseRel.y);
}

void Knob::onDragEnd() {
	guiCursorUnlock();
}


} // namespace rack
