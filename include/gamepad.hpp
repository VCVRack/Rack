#pragma once

#include "util/common.hpp"
#include "midi.hpp"


namespace rack {


const int GAMEPAD_DRIVER = -10;


struct GamepadInputDevice : MidiInputDevice {
	int device;
};


struct GamepadInputDriver : MidiInputDriver {
	GamepadInputDevice gamepadInputDevices[16];

	GamepadInputDriver();
	int getDeviceCount() override;
	std::string getDeviceName(int device) override;
	MidiInputDevice *getDevice(int device) override;
};


void gamepadStep();
MidiInputDriver *gamepadGetInputDriver();


} // namespace rack
