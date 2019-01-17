#pragma once
#include "common.hpp"
#include "midi.hpp"
#include <map>

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <rtmidi/RtMidi.h>
#pragma GCC diagnostic pop


namespace rack {


struct RtMidiInputDevice : midi::InputDevice {
	RtMidiIn *rtMidiIn;

	RtMidiInputDevice(int driverId, int deviceId);
	~RtMidiInputDevice();
};


struct RtMidiDriver : midi::Driver {
	int driverId;
	/** Just for querying MIDI driver information */
	RtMidiIn *rtMidiIn;
	RtMidiOut *rtMidiOut;
	std::map<int, RtMidiInputDevice*> devices;

	RtMidiDriver(int driverId);
	~RtMidiDriver();
	std::string getName() override;
	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	midi::InputDevice *subscribeInput(int deviceId, midi::Input *input) override;
	void unsubscribeInput(int deviceId, midi::Input *input) override;
};


void rtmidiInit();


} // namespace rack
