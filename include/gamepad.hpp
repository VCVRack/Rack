#pragma once
#include "common.hpp"
#include "midi.hpp"


namespace rack {
namespace gamepad {


struct InputDevice : midi::InputDevice {
	int deviceId;
	std::vector<uint8_t> ccs;
	std::vector<bool> states;
	void step();
};


struct Driver : midi::Driver {
	InputDevice devices[16];

	Driver();
	std::string getName() override {return "Gamepad";}
	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	midi::InputDevice *subscribeInput(int deviceId, midi::Input *input) override;
	void unsubscribeInput(int deviceId, midi::Input *input) override;
};


void init();
void step();


} // namespace gamepad
} // namespace rack
