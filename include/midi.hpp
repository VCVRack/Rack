#pragma once

#include "util.hpp"
#include <queue>
#include <vector>
#include <jansson.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "rtmidi/RtMidi.h"
#pragma GCC diagnostic pop


namespace rack {


struct MidiMessage {
	double time;
	std::vector<uint8_t> data;
};


struct MidiIO {
	int device = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;
	RtMidi *rtMidi = NULL;

	virtual ~MidiIO() {}
	int getDeviceCount();
	std::string getDeviceName(int device);
	void openDevice(int device);
	/** Returns whether the audio stream is open and running */
	bool isActive();
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


struct MidiInput : MidiIO {
	RtMidiIn *rtMidiIn = NULL;
	MidiInput();
	~MidiInput();
	virtual void onMessage(const MidiMessage &message) {}
};


struct MidiInputQueue : MidiInput {
	std::queue<MidiMessage> messageQueue;
	void onMessage(const MidiMessage &message) override;
};


struct MidiOutput : MidiIO {
	RtMidiOut *rtMidiOut = NULL;
	MidiOutput();
	~MidiOutput();
};


} // namespace rack
