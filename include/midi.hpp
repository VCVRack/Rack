#pragma once

#include "util/common.hpp"
#include <vector>
#include <queue>
#include <set>
#include <jansson.h>


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

////////////////////
// MidiIODevice
////////////////////

struct MidiIODevice {
	virtual ~MidiIODevice() {}
};

struct MidiInput;

struct MidiInputDevice : MidiIODevice {
	std::set<MidiInput*> subscribed;
	void subscribe(MidiInput *midiInput);
	void unsubscribe(MidiInput *midiInput);
	void onMessage(MidiMessage message);
};

struct MidiOutputDevice : MidiIODevice {
	// TODO
};

////////////////////
// MidiIODriver
////////////////////

struct MidiIODriver {
	virtual ~MidiIODriver() {}
	virtual int getDeviceCount() = 0;
	virtual std::string getDeviceName(int device) = 0;
};

struct MidiInputDriver : MidiIODriver {
	virtual MidiInputDevice *getDevice(int device) = 0;
};

struct MidiOutputDriver : MidiIODriver {
	virtual MidiOutputDevice *getDevice(int device) = 0;
};

////////////////////
// MidiIO
////////////////////

struct MidiIO {
	int driver = -1;
	int device = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;

	virtual ~MidiIO() {}

	std::vector<int> getDrivers();
	std::string getDriverName(int driver);
	virtual void setDriver(int driver) = 0;

	virtual int getDeviceCount() = 0;
	virtual std::string getDeviceName(int device) = 0;
	virtual void setDevice(int device) = 0;

	std::string getChannelName(int channel);
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


struct MidiInput : MidiIO {
	/** Not owned */
	MidiInputDriver *midiInputDriver = NULL;
	/** Not owned, must unsubscribe when destructed */
	MidiInputDevice *midiInputDevice = NULL;

	MidiInput();
	~MidiInput();

	void setDriver(int driver) override;
	int getDeviceCount() override;
	std::string getDeviceName(int device) override;
	void setDevice(int device) override;

	virtual void onMessage(MidiMessage message) {}
};


struct MidiInputQueue : MidiInput {
	int queueSize = 8192;
	// TODO Switch to RingBuffer
	std::queue<MidiMessage> queue;
	void onMessage(MidiMessage message) override;
	/** If a MidiMessage is available, writes `message` and return true */
	bool shift(MidiMessage *message);
};


struct MidiOutput : MidiIO {
	/** Not owned */
	MidiOutputDriver *midiOutputDriver = NULL;
	/** Not owned, must unsubscribe when destructed */
	MidiOutputDevice *midiOutputDevice = NULL;
	MidiOutput();
	~MidiOutput();
	void setDriver(int driver) override;
	void setDevice(int device) override;
};


} // namespace rack
