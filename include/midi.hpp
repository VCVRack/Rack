#pragma once

#include "util/common.hpp"
#include <queue>
#include <vector>
#include <jansson.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "rtmidi/RtMidi.h"
#pragma GCC diagnostic pop


namespace rack {


struct MidiMessage {
	uint8_t cmd = 0x00;
	uint8_t data1 = 0x00;
	uint8_t data2 = 0x00;

	uint8_t channel() {
		return cmd & 0xf;
	}
	uint8_t status() {
		return (cmd >> 4) & 0xf;
	}
	uint8_t note() {
		return data1 & 0x7f;
	}
	uint8_t value() {
		return data2 & 0x7f;
	}
};


struct MidiIO {
	int driver = -1;
	int device = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;
	RtMidi *rtMidi = NULL;
	/** Cached */
	std::string deviceName;

	virtual ~MidiIO() {}
	std::vector<int> getDrivers();
	std::string getDriverName(int driver);
	virtual void setDriver(int driver) {}

	int getDeviceCount();
	std::string getDeviceName(int device);
	virtual void setDevice(int device) {}

	std::string getChannelName(int channel);
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


struct MidiInput : MidiIO {
	RtMidiIn *rtMidiIn = NULL;
	MidiInput();
	~MidiInput();
	void setDriver(int driver) override;
	void setDevice(int device) override;
	virtual void onMessage(MidiMessage message) {}
};


struct MidiInputQueue : MidiInput {
	int queueSize = 8192;
	std::queue<MidiMessage> queue;
	void onMessage(MidiMessage message) override;
	/** If a MidiMessage is available, writes `message` and return true */
	bool shift(MidiMessage *message);
};


struct MidiOutput : MidiIO {
	RtMidiOut *rtMidiOut = NULL;
	MidiOutput();
	~MidiOutput();
	void setDriver(int driver) override;
	void setDevice(int device) override;
};


} // namespace rack
