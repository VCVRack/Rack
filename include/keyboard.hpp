#pragma once
#include "common.hpp"
#include "midi.hpp"
#include <map>


namespace rack {
namespace keyboard {


struct InputDevice : MidiInputDevice {
	int octave = 5;
	std::map<int, int> pressedNotes;
	void onKeyPress(int key);
	void onKeyRelease(int key);
};


struct Driver : MidiDriver {
	InputDevice device;
	std::string getName() override {return "Computer keyboard";}

	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) override;
	void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) override;
};


void init();
void press(int key);
void release(int key);


} // namespace keyboard
} // namespace rack
