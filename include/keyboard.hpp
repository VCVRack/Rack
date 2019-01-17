#pragma once
#include "common.hpp"
#include "midi.hpp"
#include <map>


namespace rack {
namespace keyboard {


struct InputDevice : midi::InputDevice {
	int octave = 5;
	std::map<int, int> pressedNotes;
	void onKeyPress(int key);
	void onKeyRelease(int key);
};


struct Driver : midi::Driver {
	InputDevice device;
	std::string getName() override {return "Computer keyboard";}

	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	midi::InputDevice *subscribeInput(int deviceId, midi::Input *input) override;
	void unsubscribeInput(int deviceId, midi::Input *input) override;
};


void init();
void press(int key);
void release(int key);


} // namespace keyboard
} // namespace rack
