#include "gamepad.hpp"
#include "util/common.hpp"
#include <GLFW/glfw3.h>


namespace rack {


GamepadInputDriver::GamepadInputDriver() {
	for (int i = 0; i < 16; i++) {
		gamepadInputDevices[i].device = i;
	}
}

int GamepadInputDriver::getDeviceCount() {
	return 16;
}

std::string GamepadInputDriver::getDeviceName(int device) {
	assert(0 <= device && device < 16);
	const char *name = glfwGetJoystickName(device);
	if (name) {
		return name;
	}
	return stringf("Gamepad %d (unavailable)", device + 1);
}

MidiInputDevice *GamepadInputDriver::getDevice(int device) {
	assert(0 <= device && device < 16);
	return &gamepadInputDevices[device];
}


static GamepadInputDriver *gamepadInputDriver = NULL;


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

MidiInputDriver *gamepadGetInputDriver() {
	if (!gamepadInputDriver) {
		gamepadInputDriver = new GamepadInputDriver();
	}
	return gamepadInputDriver;
}


} // namespace rack
