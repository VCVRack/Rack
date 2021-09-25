#pragma once
#include <vector>
#include <set>

#include <jansson.h>

#include <common.hpp>
#include <context.hpp>


namespace rack {
/** Abstraction for all MIDI drivers in Rack */
namespace midi {


struct Message {
	/** Initialized to 3 empty bytes. */
	std::vector<uint8_t> bytes;
	/** The Engine frame timestamp of the Message.
	For output messages, the frame when the message was generated.
	For input messages, the frame when it is intended to be processed.
	-1 for undefined, to be sent or processed immediately.
	*/
	int64_t frame = -1;

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

	std::string toString() const;

	int64_t getFrame() const {
		return frame;
	}

	void setFrame(int64_t frame) {
		this->frame = frame;
	}
};

////////////////////
// Driver
////////////////////

struct InputDevice;
struct Input;
struct OutputDevice;
struct Output;

/** Wraps a MIDI driver API containing any number of MIDI devices.
*/
struct Driver {
	virtual ~Driver() {}
	/** Returns the name of the driver. E.g. "ALSA". */
	virtual std::string getName() {
		return "";
	}
	/** Returns a list of all input device IDs that can be subscribed to. */
	virtual std::vector<int> getInputDeviceIds() {
		return {};
	}
	/** Returns the default device to use when the driver is selected, or -1 for none. */
	virtual int getDefaultInputDeviceId() {
		return -1;
	}
	/** Returns the name of an input device without obtaining it. */
	virtual std::string getInputDeviceName(int deviceId) {
		return "";
	}
	/** Adds the given port as a reference holder of a device and returns the it.
	Creates the Device if no ports are subscribed before calling.
	*/
	virtual InputDevice* subscribeInput(int deviceId, Input* input) {
		return NULL;
	}
	/** Removes the give port as a reference holder of a device.
	Deletes the Device if no ports are subscribed after calling.
	*/
	virtual void unsubscribeInput(int deviceId, Input* input) {}

	// The following behave identically as the above methods except for outputs.

	virtual std::vector<int> getOutputDeviceIds() {
		return {};
	}
	virtual int getDefaultOutputDeviceId() {
		return -1;
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

/** A single MIDI device of a driver API.

Modules and the UI should not interact with this API directly. Use Port instead.

Methods throw `rack::Exception` if the driver API has an exception.
*/
struct Device {
	virtual ~Device() {}
	virtual std::string getName() {
		return "";
	}
};

struct InputDevice : Device {
	std::set<Input*> subscribed;
	/** Not public. Use Driver::subscribeInput(). */
	void subscribe(Input* input);
	/** Not public. Use Driver::unsubscribeInput(). */
	void unsubscribe(Input* input);
	/** Called when a MIDI message is received from the device. */
	void onMessage(const Message& message);
};

struct OutputDevice : Device {
	std::set<Output*> subscribed;
	/** Not public. Use Driver::subscribeOutput(). */
	void subscribe(Output* output);
	/** Not public. Use Driver::unsubscribeOutput(). */
	void unsubscribe(Output* output);
	/** Sends a MIDI message to the device. */
	virtual void sendMessage(const Message& message) {}
};

////////////////////
// Port
////////////////////

/** A handle to a Device, typically owned by modules to have shared access to a single Device.

All Port methods safely wrap Drivers methods.
That is, if the active Device throws a `rack::Exception`, it is caught and logged inside all Port methods, so they do not throw exceptions.

Use Input or Output subclasses in your module, not Port directly.
*/
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

	Driver* getDriver();
	int getDriverId();
	void setDriverId(int driverId);

	Device* getDevice();
	virtual std::vector<int> getDeviceIds() = 0;
	virtual int getDefaultDeviceId() = 0;
	int getDeviceId();
	virtual void setDeviceId(int deviceId) = 0;
	virtual std::string getDeviceName(int deviceId) = 0;

	virtual std::vector<int> getChannels() = 0;
	int getChannel();
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

	std::vector<int> getDeviceIds() override;
	int getDefaultDeviceId() override;
	void setDeviceId(int deviceId) override;
	std::string getDeviceName(int deviceId) override;

	std::vector<int> getChannels() override;

	virtual void onMessage(const Message& message) {}
};


/** An Input port that stores incoming MIDI messages and releases them when ready according to their frame timestamp.
*/
struct InputQueue : Input {
	struct Internal;
	Internal* internal;

	InputQueue();
	~InputQueue();
	void onMessage(const Message& message) override;
	/** Pops and returns the next message (by setting `messageOut`) if its frame timestamp is `maxFrame` or earlier.
	Returns whether a message was returned.
	*/
	bool tryPop(Message* messageOut, int64_t maxFrame);
	size_t size();
};


struct Output : Port {
	/** Not owned */
	OutputDevice* outputDevice = NULL;

	Output();
	~Output();
	void reset();

	std::vector<int> getDeviceIds() override;
	int getDefaultDeviceId() override;
	void setDeviceId(int deviceId) override;
	std::string getDeviceName(int deviceId) override;

	std::vector<int> getChannels() override;

	void sendMessage(const Message& message);
};


void init();
void destroy();
/** Registers a new MIDI driver. Takes pointer ownership. */
void addDriver(int driverId, Driver* driver);
std::vector<int> getDriverIds();
Driver* getDriver(int driverId);


} // namespace midi
} // namespace rack
