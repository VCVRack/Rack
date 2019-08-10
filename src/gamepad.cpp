#include <gamepad.hpp>
#include <midi.hpp>
#include <string.hpp>
#include <window.hpp>


namespace rack {
namespace gamepad {


struct Driver;


static const int DRIVER = -10;
static Driver* driver = NULL;


struct InputDevice : midi::InputDevice {
	int deviceId;
	int8_t ccs[128] = {};

	void step() {
		if (!glfwJoystickPresent(deviceId))
			return;

		// Get gamepad state
		int numAxes;
		const float* axes = glfwGetJoystickAxes(deviceId, &numAxes);
		int numButtons;
		const unsigned char* buttons = glfwGetJoystickButtons(deviceId, &numButtons);

		// Convert axes and buttons to MIDI CC
		int numCcs = std::min(numAxes + numButtons, 128);
		for (int i = 0; i < numCcs; i++) {
			// Allow CC value to go negative, but clamp at -127 instead of -128 for symmetry
			int8_t cc;
			if (i < numAxes) {
				// Axis
				cc = math::clamp((int) std::round(axes[i] * 127), -127, 127);
			}
			else {
				// Button
				cc = buttons[i - numAxes] ? 127 : 0;
			}

			if (cc == ccs[i])
				continue;
			ccs[i] = cc;

			// Send MIDI message
			midi::Message msg;
			msg.setStatus(0xb);
			msg.setNote(i);
			// Allow 8th bit to be set
			msg.bytes[2] = cc;
			onMessage(msg);
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

	std::string getInputDeviceName(int deviceId) override {
		if (!(0 <= deviceId && deviceId < 16))
			return "";

		const char* name = glfwGetJoystickName(deviceId);
		if (name) {
			return name;
		}
		return string::f(" %d (unavailable)", deviceId + 1);
	}

	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		if (!(0 <= deviceId && deviceId < 16))
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
	if (!driver)
		return;
	for (int i = 0; i < 16; i++) {
		if (glfwJoystickPresent(i)) {
			driver->devices[i].step();
		}
	}
}


} // namespace gamepad
} // namespace rack
