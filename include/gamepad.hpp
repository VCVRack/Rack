#pragma once
#include "common.hpp"
#include "midi.hpp"


namespace rack {
namespace gamepad {


struct InputDevice : MidiInputDevice {
	int deviceId;
	std::vector<uint8_t> ccs;
	std::vector<bool> states;
	void step();
};


struct Driver : MidiDriver {
	InputDevice devices[16];

	Driver();
	std::string getName() override {return "Gamepad";}
	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) override;
	void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) override;
};


void init();
void step();


} // namespace gamepad
} // namespace rack
