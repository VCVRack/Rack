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

	RtMidiInputDriver(int driver);
	~RtMidiInputDriver();
	int getDeviceCount() override;
	std::string getDeviceName(int device) override;
	MidiInputDevice *getDevice(int device) override;
};


struct RtMidiInputDevice : MidiInputDevice {
	RtMidiIn *rtMidiIn;
	/** Cached */
	std::string deviceName;

	RtMidiInputDevice(int driver, int device);
	~RtMidiInputDevice();
};


std::vector<int> rtmidiGetDrivers();
MidiInputDriver *rtmidiGetInputDriver(int driver);


} // namespace rack
