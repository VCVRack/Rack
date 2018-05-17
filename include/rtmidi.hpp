#pragma once

#include "util/common.hpp"
#include "midi.hpp"

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include "rtmidi/RtMidi.h"
#pragma GCC diagnostic pop


namespace rack {


struct RtMidiInputDriver : MidiInputDriver {
	/** Just for querying MIDI driver information */
	RtMidiIn *rtMidiIn;

	RtMidiInputDriver(int driverId);
	~RtMidiInputDriver();
	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	MidiInputDevice *getDevice(int deviceId) override;
};


struct RtMidiInputDevice : MidiInputDevice {
	RtMidiIn *rtMidiIn;
	/** Cached */
	std::string deviceName;

	RtMidiInputDevice(int driverId, int deviceId);
	~RtMidiInputDevice();
};


std::vector<int> rtmidiGetDrivers();
MidiInputDriver *rtmidiGetInputDriver(int driverId);


} // namespace rack
