#include "gamepad.hpp"
#include <GLFW/glfw3.h>


namespace rack {


static const int GAMEPAD_DRIVER = -10;
static GamepadDriver *driver = NULL;


void GamepadInputDevice::step() {
	if (!glfwJoystickPresent(deviceId))
		return;
	// Get gamepad state
	int numAxes;
	const float *axes = glfwGetJoystickAxes(deviceId, &numAxes);
	int numButtons;
	const unsigned char *buttons = glfwGetJoystickButtons(deviceId, &numButtons);

	// Convert axes to MIDI CC
	ccs.resize(numAxes);
	for (int i = 0; i < numAxes; i++) {
		// Allow CC value to go negative, but clamp at -127 instead of -128 for symmetry
		int8_t cc = clamp((int) (axes[i] * 127), -127, 127);
		if (cc != ccs[i]) {
			ccs[i] = cc;

			// Send MIDI message
			MidiMessage msg;
			// MIDI channel 1
			msg.cmd = (0xb << 4) | 0;
			msg.data1 = i;
			msg.data2 = ccs[i];
			onMessage(msg);
		}
	}

	// Convert buttons to MIDI notes
	states.resize(numButtons);
	for (int i = 0; i < numButtons; i++) {
		bool state = !!buttons[i];
		if (state != states[i]) {
			states[i] = state;

			MidiMessage msg;
			msg.cmd = ((state ? 0x9 : 0x8) << 4);
			msg.data1 = i;
			msg.data2 = 127;
			onMessage(msg);
		}
	}
}


GamepadDriver::GamepadDriver() {
	for (int i = 0; i < 16; i++) {
		devices[i].deviceId = i;
	}
}

std::vector<int> GamepadDriver::getInputDeviceIds() {
	std::vector<int> deviceIds;
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			deviceIds.push_back(i);
		}
	}
	return deviceIds;
}

std::string GamepadDriver::getInputDeviceName(int deviceId) {
	if (!(0 <= deviceId && deviceId < 16))
		return "";

	const char *name = glfwGetJoystickName(deviceId);
	if (name) {
		return name;
	}
	return stringf("Gamepad %d (unavailable)", deviceId + 1);
}

MidiInputDevice *GamepadDriver::subscribeInputDevice(int deviceId, MidiInput *midiInput) {
	if (!(0 <= deviceId && deviceId < 16))
		return NULL;

	devices[deviceId].subscribe(midiInput);
	return &devices[deviceId];
}

void GamepadDriver::unsubscribeInputDevice(int deviceId, MidiInput *midiInput) {
	if (!(0 <= deviceId && deviceId < 16))
		return;

	devices[deviceId].unsubscribe(midiInput);
}


void gamepadInit() {
	driver = new GamepadDriver();
	midiDriverAdd(GAMEPAD_DRIVER, driver);
}

void gamepadStep() {
	if (!driver)
		return;
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			driver->devices[i].step();
		}
	}
}


} // namespace rack
