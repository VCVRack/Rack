#include "gamepad.hpp"
#include "util/common.hpp"
#include <GLFW/glfw3.h>


namespace rack {


void GamepadInputDevice::step() {
	if (!glfwJoystickPresent(deviceId))
		return;
	// Get gamepad state
	int numAxes;
	const float *axes = glfwGetJoystickAxes(deviceId, &numAxes);
	int numButtons;
	const unsigned char *buttons = glfwGetJoystickButtons(deviceId, &numButtons);
	// Update state
	ccs.resize(numAxes + numButtons);
	for (int i = 0; i < numAxes; i++) {
		// Allow CC value to go negative, but clamp at -127 instead of -128 for symmetry
		updateCc(i, clamp((int) (axes[i] * 127), -127, 127));
	}
	for (int i = 0; i < numButtons; i++) {
		updateCc(numAxes + i, buttons[i] ? 127 : 0);
	}
}

void GamepadInputDevice::updateCc(int index, uint8_t cc) {
	if ((int) ccs.size() < index)
		return;

	if (cc != ccs[index]) {
		ccs[index] = cc;

		// Send MIDI message
		MidiMessage msg;
		// MIDI channel 1
		msg.cmd = (0xb << 4) | 0;
		msg.data1 = index;
		msg.data2 = ccs[index];
		onMessage(msg);
	}
}


GamepadInputDriver::GamepadInputDriver() {
	for (int i = 0; i < 16; i++) {
		devices[i].deviceId = i;
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

	return &devices[deviceId];
}


static GamepadInputDriver *driver = NULL;


void gamepadStep() {
	// Check if the driver has been instantiated
	if (!driver)
		return;

	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			driver->devices[i].step();
		}
	}
}

MidiInputDriver *gamepadGetInputDriver() {
	if (!driver) {
		driver = new GamepadInputDriver();
	}
	return driver;
}


} // namespace rack
