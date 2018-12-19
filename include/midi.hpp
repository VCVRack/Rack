#pragma once
#include "common.hpp"
#include <vector>
#include <queue>
#include <set>
#include <jansson.h>


namespace rack {
namespace midi {


struct Message {
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
// Device
////////////////////

struct Device {
	virtual ~Device() {}
};

struct Input;

struct InputDevice : Device {
	std::set<Input*> subscribed;
	void subscribe(Input *input);
	void unsubscribe(Input *input);
	void onMessage(Message message);
};

struct OutputDevice : Device {
	// TODO
};

////////////////////
// Driver
////////////////////

struct Driver {
	virtual ~Driver() {}
	virtual std::string getName() {return "";}

	virtual std::vector<int> getInputDeviceIds() {return {};}
	virtual std::string getInputDeviceName(int deviceId) {return "";}
	virtual InputDevice *subscribeInputDevice(int deviceId, Input *input) {return NULL;}
	virtual void unsubscribeInputDevice(int deviceId, Input *input) {}

	// virtual std::vector<int> getOutputDeviceIds() = 0;
	// virtual std::string getOutputDeviceName(int deviceId) = 0;
	// virtual OutputDevice *subscribeOutputDevice(int deviceId, Output *midiOutput) = 0;
	// virtual void unsubscribeOutputDevice(int deviceId, Output *midiOutput) = 0;
};

////////////////////
// IO
////////////////////

struct IO {
	int driverId = -1;
	int deviceId = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;
	/** Not owned */
	Driver *driver = NULL;

	virtual ~IO();

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


struct Input : IO {
	Input();
	~Input();

	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	void setDeviceId(int deviceId) override;
	virtual void onMessage(Message message) {}
};


struct InputQueue : Input {
	int queueMaxSize = 8192;
	std::queue<Message> queue;
	void onMessage(Message message) override;
	/** If a Message is available, writes `message` and return true */
	bool shift(Message *message);
};


struct Output : IO {
	Output();
	~Output();
	void setDeviceId(int deviceId) override;
};


void init();
void destroy();
/** Registers a new MIDI driver. Takes pointer ownership. */
void addDriver(int driverId, Driver *driver);


} // namespace midi
} // namespace rack
