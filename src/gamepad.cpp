#include "gamepad.hpp"
#include "util/common.hpp"
#include <GLFW/glfw3.h>


namespace rack {


void gamepadStep() {
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			const char *name = glfwGetJoystickName(i);
			int numButtons;
			const unsigned char *buttons = glfwGetJoystickButtons(i, &numButtons);
			int numAxes;
			const float *axes = glfwGetJoystickAxes(i, &numAxes);
			debug("%d %s", i, name);
		}
	}
}


} // namespace rack
