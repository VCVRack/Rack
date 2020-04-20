#pragma once
#include <vector>
#include <queue>
#include <set>

#include <jansson.h>

#include <common.hpp>
#include <context.hpp>


namespace rack {


/** MIDI driver
*/
namespace midi {


struct Message {
	/** Initialized to 3 empty bytes. */
	std::vector<uint8_t> bytes;
	/** Timestamp of MIDI message in nanoseconds. Negative if not set. */
	int64_t timestamp = -1;

	Message() : bytes(3) {}

	int getSize() const {
		return bytes.size();
	}
	void setSize(int size) {
		bytes.resize(size);
	}

	uint8_t getChannel() const {
		if (bytes.size() < 1)
			return 0;
		return bytes[0] & 0xf;
	}
	void setChannel(uint8_t channel) {
		if (bytes.size() < 1)
			return;
		bytes[0] = (bytes[0] & 0xf0) | (channel & 0xf);
	}

	uint8_t getStatus() const {
		if (bytes.size() < 1)
			return 0;
		return bytes[0] >> 4;
	}
	void setStatus(uint8_t status) {
		if (bytes.size() < 1)
			return;
		bytes[0] = (bytes[0] & 0xf) | (status << 4);
	}

	uint8_t getNote() const {
		if (bytes.size() < 2)
			return 0;
		return bytes[1];
	}
	void setNote(uint8_t note) {
		if (bytes.size() < 2)
			return;
		bytes[1] = note & 0x7f;
	}

	uint8_t getValue() const {
		if (bytes.size() < 3)
			return 0;
		return bytes[2];
	}
	void setValue(uint8_t value) {
		if (bytes.size() < 3)
			return;
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
	virtual std::string getName() {
		return "";
	}
};

struct InputDevice : Device {
	std::set<Input*> subscribed;
	void subscribe(Input* input);
	void unsubscribe(Input* input);
	void onMessage(const Message &message);
};

struct OutputDevice : Device {
	std::set<Output*> subscribed;
	void subscribe(Output* input);
	void unsubscribe(Output* input);
	virtual void sendMessage(const Message &message) {}
};

////////////////////
// Port
////////////////////

struct Port {
	/** For MIDI output, the channel to automatically set outbound messages.
	If -1, the channel is not overwritten and must be set by MIDI generator.

	For MIDI input, messages will be filtered by the channel.
	If -1, all MIDI channels pass through.
	*/
	int channel = -1;

	// private
	int driverId = -1;
	int deviceId = -1;
	/** Not owned */
	Driver* driver = NULL;
	Device* device = NULL;
	Context* context;

	Port();
	virtual ~Port();

	Driver* getDriver() {
		return driver;
	}
	int getDriverId() {
		return driverId;
	}
	void setDriverId(int driverId);

	Device* getDevice() {
		return device;
	}
	virtual std::vector<int> getDeviceIds() = 0;
	int getDeviceId() {
		return deviceId;
	}
	virtual void setDeviceId(int deviceId) = 0;
	virtual std::string getDeviceName(int deviceId) = 0;

	virtual std::vector<int> getChannels() = 0;
	int getChannel() {
		return channel;
	}
	void setChannel(int channel);
	std::string getChannelName(int channel);

	json_t* toJson();
	void fromJson(json_t* rootJ);
};


struct Input : Port {
	/** Not owned */
	InputDevice* inputDevice = NULL;

	Input();
	~Input();
	void reset();

	std::vector<int> getDeviceIds() override {
		if (driver)
			return driver->getInputDeviceIds();
		return {};
	}
	void setDeviceId(int deviceId) override;
	std::string getDeviceName(int deviceId) override {
		if (driver)
			return driver->getInputDeviceName(deviceId);
		return "";
	}

	std::vector<int> getChannels() override;

	virtual void onMessage(const Message &message) {}
};


struct InputQueue : Input {
	int queueMaxSize = 8192;
	std::queue<Message> queue;
	void onMessage(const Message &message) override;
};


struct Output : Port {
	/** Not owned */
	OutputDevice* outputDevice = NULL;

	Output();
	~Output();
	void reset();

	std::vector<int> getDeviceIds() override {
		if (driver)
			return driver->getOutputDeviceIds();
		return {};
	}
	void setDeviceId(int deviceId) override;
	std::string getDeviceName(int deviceId) override {
		if (driver)
			return driver->getInputDeviceName(deviceId);
		return "";
	}

	std::vector<int> getChannels() override;

	void sendMessage(const Message &message);
};


void init();
void destroy();
/** Registers a new MIDI driver. Takes pointer ownership. */
void addDriver(int driverId, Driver* driver);
std::vector<int> getDriverIds();
Driver* getDriver(int driverId);


} // namespace midi
} // namespace rack
