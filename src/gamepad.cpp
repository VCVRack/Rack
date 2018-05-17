#include "gamepad.hpp"
#include "util/common.hpp"
#include <GLFW/glfw3.h>


namespace rack {


GamepadInputDriver::GamepadInputDriver() {
	for (int i = 0; i < 16; i++) {
		gamepadInputDevices[i].deviceId = i;
	}
}

std::vector<int> GamepadInputDriver::getDeviceIds() {
	std::vector<int> deviceIds;
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			deviceIds.push_back(i);
		}
	}
	return deviceIds;
}

std::string GamepadInputDriver::getDeviceName(int deviceId) {
	if (!(0 <= deviceId && deviceId < 16))
		return "";

	const char *name = glfwGetJoystickName(deviceId);
	if (name) {
		return name;
	}
	return stringf("Gamepad %d (unavailable)", deviceId + 1);
}

MidiInputDevice *GamepadInputDriver::getDevice(int deviceId) {
	if (!(0 <= deviceId && deviceId < 16))
		return NULL;

	return &gamepadInputDevices[deviceId];
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
