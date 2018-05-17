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


struct RtMidiInputDevice : MidiInputDevice {
	RtMidiIn *rtMidiIn;
	/** Cached */
	std::string deviceName;

	RtMidiInputDevice(int driver, int device);
	~RtMidiInputDevice();
};


std::vector<int> rtmidiGetDrivers();
int rtmidiGetDeviceCount(int driver);
std::string rtmidiGetDeviceName(int driver, int device);
RtMidiInputDevice *rtmidiGetDevice(int driver, int device);


} // namespace rack
