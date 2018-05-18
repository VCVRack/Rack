#pragma once

#include "util/common.hpp"
#include "midi.hpp"


namespace rack {


const int KEYBOARD_DRIVER = -11;


struct KeyboardInputDevice : MidiInputDevice {
	int octave = 5;
	void processKey(int key, bool released);
};


struct KeyboardInputDriver : MidiInputDriver {
	KeyboardInputDevice device;

	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	MidiInputDevice *getDevice(int deviceId) override;
};


void keyboardPress(int key);
void keyboardRelease(int key);
KeyboardInputDriver *keyboardGetInputDriver();


} // namespace rack
