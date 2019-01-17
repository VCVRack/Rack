#include "gamepad.hpp"
#include "midi.hpp"
#include "string.hpp"
#include "window.hpp"


namespace rack {
namespace gamepad {


struct Driver;


static const int DRIVER = -10;
static Driver *driver = NULL;


struct InputDevice : midi::InputDevice {
	int deviceId;
	std::vector<uint8_t> ccs;
	std::vector<bool> states;

	void step() {
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
			int8_t cc = math::clamp((int) std::round(axes[i] * 127), -127, 127);
			if (cc != ccs[i]) {
				ccs[i] = cc;

				// Send MIDI message
				midi::Message msg;
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

				midi::Message msg;
				msg.cmd = ((state ? 0x9 : 0x8) << 4);
				msg.data1 = i;
				msg.data2 = 127;
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

	std::string getName() override {return "Gamepad";}

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

		const char *name = glfwGetJoystickName(deviceId);
		if (name) {
			return name;
		}
		return string::f(" %d (unavailable)", deviceId + 1);
	}

	midi::InputDevice *subscribeInput(int deviceId, midi::Input *input) override {
		if (!(0 <= deviceId && deviceId < 16))
			return NULL;

		devices[deviceId].subscribe(input);
		return &devices[deviceId];
	}

	void unsubscribeInput(int deviceId, midi::Input *input) override {
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
