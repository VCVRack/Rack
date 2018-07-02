#pragma once

#include "midi.hpp"
#include <map>


namespace rack {

#define VST_DRIVER  (-0x4489)


struct VSTMidiInputDevice : MidiInputDevice {

	VSTMidiInputDevice(int driverId, int deviceId);
	~VSTMidiInputDevice();
};


struct VSTMidiDriver : MidiDriver {
   VSTMidiInputDevice *device;

	VSTMidiDriver(int driverId);
	~VSTMidiDriver();
	std::string getName() override;
	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) override;
	void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) override;
};


void vstmidiInit();


} // namespace rack
