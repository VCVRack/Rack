#pragma once

#include "util/common.hpp"
#include "midi.hpp"


namespace rack {


const int GAMEPAD_DRIVER = -10;


struct GamepadInputDevice : MidiInputDevice {
	int deviceId;
	std::vector<uint8_t> ccs;
	void step();
	void updateCc(int index, uint8_t cc);
};


struct GamepadInputDriver : MidiInputDriver {
	GamepadInputDevice devices[16];

	GamepadInputDriver();
	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	MidiInputDevice *getDevice(int deviceId) override;
};


void gamepadStep();
MidiInputDriver *gamepadGetInputDriver();


} // namespace rack
