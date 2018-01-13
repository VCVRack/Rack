#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "rtmidi/RtMidi.h"
#pragma GCC diagnostic pop


namespace rack {


struct MidiIO {
	RtMidi *midi;
	int port = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;

	virtual ~MidiIO() {}
	virtual int getPortCount();
	virtual std::string getPortName(int port);
	virtual void openPort(int port);
};


struct MidiInput : MidiIO {
	MidiInput();
	~MidiInput();
};


struct MidiOutput : MidiIO {
	MidiOutput();
	~MidiOutput();
};


} // namespace rack
