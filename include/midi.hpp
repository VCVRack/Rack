#pragma once
#include <common.hpp>
#include <vector>
#include <queue>
#include <set>
#include <jansson.h>


namespace rack {


/** MIDI driver
*/
namespace midi {


struct Message {
	uint8_t size = 3;
	uint8_t bytes[3] = {};

	void setSize(uint8_t size) {
		assert(size <= 3);
		this->size = size;
	}
	uint8_t getChannel() {
		return bytes[0] & 0xf;
	}
	void setChannel(uint8_t channel) {
		bytes[0] = (bytes[0] & 0xf0) | (channel & 0xf);
	}
	uint8_t getStatus() {
		return bytes[0] >> 4;
	}
	void setStatus(uint8_t status) {
		bytes[0] = (bytes[0] & 0xf) | (status << 4);
	}
	uint8_t getNote() {
		return bytes[1];
	}
	void setNote(uint8_t note) {
		bytes[1] = note & 0x7f;
	}
	uint8_t getValue() {
		return bytes[2];
	}
	void setValue(uint8_t value) {
		bytes[2] = value & 0x7f;
	}
};

////////////////////
// Driver
////////////////////

struct InputDevice;
struct Input;
struct OutputDevice;
struct Output;

struct Driver {
	virtual ~Driver() {}
	virtual std::string getName() {
		return "";
	}

	virtual std::vector<int> getInputDeviceIds() {
		return {};
	}
	virtual std::string getInputDeviceName(int deviceId) {
		return "";
	}
	virtual InputDevice* subscribeInput(int deviceId, Input* input) {
		return NULL;
	}
	virtual void unsubscribeInput(int deviceId, Input* input) {}

	virtual std::vector<int> getOutputDeviceIds() {
		return {};
	}
	virtual std::string getOutputDeviceName(int deviceId) {
		return "";
	}
	virtual OutputDevice* subscribeOutput(int deviceId, Output* output) {
		return NULL;
	}
	virtual void unsubscribeOutput(int deviceId, Output* output) {}
};

////////////////////
// Device
////////////////////

struct Device {
	virtual ~Device() {}
};

struct InputDevice : Device {
	std::set<Input*> subscribed;
	void subscribe(Input* input);
	void unsubscribe(Input* input);
	void onMessage(Message message);
};

struct OutputDevice : Device {
	std::set<Output*> subscribed;
	void subscribe(Output* input);
	void unsubscribe(Output* input);
	virtual void sendMessage(Message message) {}
};

////////////////////
// Port
////////////////////

struct Port {
	int driverId = -1;
	int deviceId = -1;
	/* For MIDI output, the channel to output messages.
	For MIDI input, the channel to filter.
	Set to -1 to allow all MIDI channels (for input).
	Zero indexed.
	*/
	int channel = -1;
	/** Not owned */
	Driver* driver = NULL;

	/** Remember to call setDriverId(-1) in subclass destructors. */
	virtual ~Port() {}

	std::vector<int> getDriverIds();
	std::string getDriverName(int driverId);
	void setDriverId(int driverId);

	virtual std::vector<int> getDeviceIds() = 0;
	virtual std::string getDeviceName(int deviceId) = 0;
	virtual void setDeviceId(int deviceId) = 0;

	virtual std::vector<int> getChannels() = 0;
	std::string getChannelName(int channel);
	void setChannel(int channel);

	json_t* toJson();
	void fromJson(json_t* rootJ);
};


struct Input : Port {
	/** Not owned */
	InputDevice* inputDevice = NULL;

	Input();
	~Input();

	void reset();
	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	void setDeviceId(int deviceId) override;
	std::vector<int> getChannels() override;

	virtual void onMessage(Message message) {}
};


struct InputQueue : Input {
	int queueMaxSize = 8192;
	std::queue<Message> queue;
	void onMessage(Message message) override;
	/** If a Message is available, writes `message` and return true */
	bool shift(Message* message);
};


struct Output : Port {
	/** Not owned */
	OutputDevice* outputDevice = NULL;

	Output();
	~Output();

	void reset();
	std::vector<int> getDeviceIds() override;
	std::string getDeviceName(int deviceId) override;
	void setDeviceId(int deviceId) override;
	std::vector<int> getChannels() override;

	void sendMessage(Message message);
};


void init();
void destroy();
/** Registers a new MIDI driver. Takes pointer ownership. */
void addDriver(int driverId, Driver* driver);


} // namespace midi
} // namespace rack
