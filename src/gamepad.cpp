#include <gamepad.hpp>
#include <midi.hpp>
#include <string.hpp>
#include <window/Window.hpp>


namespace rack {
namespace gamepad {


struct Driver;


static const int DRIVER = -10;
static Driver* driver = NULL;


struct InputDevice : midi::InputDevice {
	int deviceId;
	int16_t ccValues[128] = {};

	std::string getName() override {
		const char* name = glfwGetJoystickName(deviceId);
		if (!name)
			return "";
		return name;
	}

	void step() {
		if (!glfwJoystickPresent(deviceId))
			return;

		// Get gamepad state
		int numAxes;
		const float* axes = glfwGetJoystickAxes(deviceId, &numAxes);
		int numButtons;
		const unsigned char* buttons = glfwGetJoystickButtons(deviceId, &numButtons);

		// Convert axes and buttons to MIDI CC
		// Unfortunately to support 14-bit MIDI CC, only the first 32 CCs can be used.
		// This could be fixed by continuing with CC 64 if more than 32 CCs are needed.
		int numCcs = std::min(numAxes + numButtons, 32);
		for (int i = 0; i < numCcs; i++) {
			// Allow CC value to go negative
			int16_t value;
			if (i < numAxes) {
				// Axis
				value = math::clamp((int) std::round(axes[i] * 0x3f80), -0x3f80, 0x3f80);
			}
			else {
				// Button
				value = buttons[i - numAxes] ? 0x3f80 : 0;
			}

			if (value == ccValues[i])
				continue;
			ccValues[i] = value;

			// Send MSB MIDI message
			midi::Message msg;
			msg.setStatus(0xb);
			msg.setNote(i);
			// Allow 8th bit to be set to allow bipolar value hack.
			msg.bytes[2] = (value >> 7);
			onMessage(msg);

			// Send LSB MIDI message for axis CCs
			if (i < numAxes) {
				midi::Message msg;
				msg.setStatus(0xb);
				msg.setNote(i + 32);
				msg.bytes[2] = (value & 0x7f);
				onMessage(msg);
			}
		}
	}
};


struct Driver : midi::Driver {
	InputDevice devices[16];

	Driver() {
		for (int i = 0; i < 16; i++) {
			devices[i].deviceId = i;
		}
	}

	std::string getName() override {
		return "Gamepad";
	}

	std::vector<int> getInputDeviceIds() override {
		std::vector<int> deviceIds;
		for (int i = 0; i < 16; i++) {
			if (glfwJoystickPresent(i)) {
				deviceIds.push_back(i);
			}
		}
		return deviceIds;
	}

	int getDefaultInputDeviceId() override {
		return 0;
	}

	std::string getInputDeviceName(int deviceId) override {
		if (!(0 <= deviceId && deviceId < 16))
			return "";

		const char* name = glfwGetJoystickName(deviceId);
		if (!name)
			return string::f("#%d (unavailable)", deviceId + 1);
		return name;
	}

	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		if (!(0 <= deviceId && deviceId < 16))
			return NULL;
		if (!glfwJoystickPresent(deviceId))
			return NULL;

		devices[deviceId].subscribe(input);
		return &devices[deviceId];
	}

	void unsubscribeInput(int deviceId, midi::Input* input) override {
		if (!(0 <= deviceId && deviceId < 16))
			return;

		devices[deviceId].unsubscribe(input);
	}
};




void init() {
	driver = new Driver;
	midi::addDriver(DRIVER, driver);
}

void step() {
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			driver->devices[i].step();
		}
	}
}


} // namespace gamepad
} // namespace rack
