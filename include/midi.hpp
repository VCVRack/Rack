#pragma once

#include "util/common.hpp"
#include <vector>
#include <queue>
#include <set>
#include <jansson.h>


namespace rack {


struct MidiMessage {
	uint8_t cmd   = 0u;
	uint8_t data1 = 0u;
	uint8_t data2 = 0u;

	uint8_t channel() const {
		return cmd & 15u;
	}
	uint8_t status() const {
		return (cmd >> 4) & 15u;
	}
	uint8_t note() const {
		return data1 & 127u;
	}
	uint8_t value() const {
		return data2 & 127u;
	}
	uint8_t getData1() const {
		return data1 & 127u;
	}
	uint8_t getData2() const {
		return data2 & 127u;
	}
};

////////////////////
// MidiDevice
////////////////////

struct MidiDevice {
	virtual ~MidiDevice() {}
};

struct MidiInput;

struct MidiInputDevice : MidiDevice {
	std::set<MidiInput*> subscribed;
	void subscribe(MidiInput *midiInput);
	void unsubscribe(MidiInput *midiInput);
	void onMessage(MidiMessage message);
};

struct MidiOutputDevice : MidiDevice {
	// TODO
};

////////////////////
// MidiDriver
////////////////////

struct MidiDriver {
	virtual ~MidiDriver() {}
	virtual std::string getName() {return "";}

	virtual std::vector<int> getInputDeviceIds() {return {};}
	virtual std::string getInputDeviceName(int deviceId) {return "";}
	virtual MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) {return NULL;}
	virtual void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) {}

	// virtual std::vector<int> getOutputDeviceIds() = 0;
	// virtual std::string getOutputDeviceName(int deviceId) = 0;
	// virtual MidiOutputDevice *subscribeOutputDevice(int deviceId, MidiOutput *midiOutput) = 0;
	// virtual void unsubscribeOutputDevice(int deviceId, MidiOutput *midiOutput) = 0;
};

////////////////////
// MidiIO
////////////////////

struct MidiIO {
	int driverId = -1;
	int deviceId = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;
	/** Not owned */
	MidiDriver *driver = NULL;

   MidiIO(void);
	virtual ~MidiIO();

	std::vector<int> getDriverIds();
	std::string getDriverName(int driverId);
	void setDriverId(int driverId);

	virtual std::vector<int> getDeviceIds() = 0;
	virtual std::string getDeviceName(int deviceId) = 0;
	virtual void setDeviceId(int deviceId) = 0;

	std::string getChannelName(int channel);
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


struct MidiInput : MidiIO {
	MidiInput();
	~MidiInput();

	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	void setDeviceId(int deviceId) override;
	virtual void onMessage(MidiMessage message) {}
};


struct MidiInputQueue : MidiInput {
	int queueMaxSize = 8192;
	std::queue<MidiMessage> queue;
	void onMessage(MidiMessage message) override;
	/** If a MidiMessage is available, writes `message` and return true */
	bool shift(MidiMessage *message);
};


struct MidiOutput : MidiIO {
	MidiOutput();
	~MidiOutput();
	void setDeviceId(int deviceId) override;
};


void midiDestroy();
/** Registers a new MIDI driver. Takes pointer ownership. */
void midiDriverAdd(int driverId, MidiDriver *driver);


} // namespace rack
