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
	// Drag slower if Ctrl is held (Command on Mac)
#ifdef ARCH_MAC
	if (guiIsKeyPressed(GLFW_KEY_LEFT_SUPER) || guiIsKeyPressed(GLFW_KEY_RIGHT_SUPER))
#else
	if (guiIsKeyPressed(GLFW_KEY_LEFT_CONTROL) || guiIsKeyPressed(GLFW_KEY_RIGHT_CONTROL))
#endif
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
