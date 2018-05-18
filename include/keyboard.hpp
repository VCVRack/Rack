#pragma once

#include "util/common.hpp"
#include "midi.hpp"


namespace rack {


const int KEYBOARD_DRIVER = -11;


struct KeyboardInputDevice : MidiInputDevice {
	int octave = 5;
	void processKey(int key, bool released);
};


struct KeyboardDriver : MidiDriver {
	KeyboardInputDevice device;
	std::string getName() override {return "Computer keyboard";}

	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) override;
	void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) override;
};


void keyboardPress(int key);
void keyboardRelease(int key);
KeyboardDriver *keyboardGetDriver();


} // namespace rack
