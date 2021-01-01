#pragma once
#include <vector>
#include <set>

#include <jansson.h>

#include <common.hpp>
#include <context.hpp>

namespace rack {


/** Audio driver
*/
namespace audio {


////////////////////
// Driver
////////////////////

struct Device;
struct Port;

/** Wraps an audio driver API containing any number of audio devices.
*/
struct Driver {
	virtual ~Driver() {}
	/** Returns the name of the driver. E.g. "ALSA". */
	virtual std::string getName() {
		return "";
	}
	/** Returns a list of all device IDs that can be subscribed to. */
	virtual std::vector<int> getDeviceIds() {
		return {};
	}
	/** Returns the name of a device without obtaining it. */
	virtual std::string getDeviceName(int deviceId) {
		return "";
	}
	/** Returns the number of inputs of a device without obtaining it. */
	virtual int getDeviceNumInputs(int deviceId) {
		return 0;
	}
	/** Returns the number of output of a device without obtaining it. */
	virtual int getDeviceNumOutputs(int deviceId) {
		return 0;
	}
	/** Returns a detailed description of the device without obtaining it.
	`offset` specifies the first channel (zero-indexed).
	E.g. "MySoundcard (1-2 in, 1-2 out)"
	*/
	std::string getDeviceDetail(int deviceId, int offset, int maxChannels);

	/** Adds the given port as a reference holder of a device and returns the it.
	Creates the Device if no ports are subscribed before calling.
	*/
	virtual Device* subscribe(int deviceId, Port* port) {
		return NULL;
	}
	/** Removes the give port as a reference holder of a device.
	Deletes the Device if no ports are subscribed after calling.
	*/
	virtual void unsubscribe(int deviceId, Port* port) {}
};

////////////////////
// Device
////////////////////

/** A single audio device of a driver API.

Modules and the UI should not interact with this API directly. Use Port instead.

Methods throw `rack::Exception` if the driver API has an exception.
*/
struct Device {
	std::set<Port*> subscribed;
	virtual ~Device() {}
	// Called by Driver::subscribe().
	void subscribe(Port* port);
	void unsubscribe(Port* port);

	virtual std::string getName() {
		return "";
	}
	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 0;
	}
	/** Returns a detailed description of the device.
	`offset` specifies the first channel (zero-indexed).
	E.g. "MySoundcard (1-2 in, 1-2 out)"
	*/
	std::string getDetail(int offset, int maxChannels);

	/** Returns a list of all valid (user-selectable) sample rates.
	The device may accept sample rates not in this list, but it *must* accept sample rates in the list.
	*/
	virtual std::set<float> getSampleRates() {
		return {};
	}
	/** Returns the current sample rate. */
	virtual float getSampleRate() {
		return 0;
	}
	/** Sets the sample rate of the device, re-opening it if needed. */
	virtual void setSampleRate(float sampleRate) {}

	/** Returns a list of all valid (user-selectable) block sizes.
	The device may accept block sizes not in this list, but it *must* accept block sizes in the list.
	*/
	virtual std::set<int> getBlockSizes() {
		return {};
	}
	/** Returns the current block size. */
	virtual int getBlockSize() {
		return 0;
	}
	/** Sets the block size of the device, re-opening it if needed. */
	virtual void setBlockSize(int blockSize) {}

	// Called by this Device class, forwards to subscribed Ports.
	void processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames);
	void onOpenStream();
	void onCloseStream();
};

////////////////////
// Port
////////////////////

/** A handle to a Device, typically owned by modules to have shared access to a single Device.

All Port methods safely wrap Drivers methods.
That is, if the active Device throws a `rack::Exception`, it is caught and logged inside all Port methods, so they do not throw exceptions.
*/
struct Port {
	/** The first channel index of the device to process. */
	int offset = 0;
	/** Maximum number of channels to process. */
	int maxChannels = 8;

	// private
	int driverId = -1;
	int deviceId = -1;
	/** Not owned */
	Driver* driver = NULL;
	Device* device = NULL;
	Context* context;

	Port();
	virtual ~Port();
	void reset();

	Driver* getDriver();
	int getDriverId();
	void setDriverId(int driverId);
	std::string getDriverName();

	Device* getDevice();
	std::vector<int> getDeviceIds();
	int getDeviceId();
	void setDeviceId(int deviceId);
	int getDeviceNumInputs(int deviceId);
	int getDeviceNumOutputs(int deviceId);
	std::string getDeviceName(int deviceId);
	std::string getDeviceDetail(int deviceId, int offset);

	std::set<float> getSampleRates();
	float getSampleRate();
	void setSampleRate(float sampleRate);

	std::set<int> getBlockSizes();
	int getBlockSize();
	void setBlockSize(int blockSize);

	int getOffset();
	void setOffset(int offset);

	int getNumInputs();
	int getNumOutputs();

	json_t* toJson();
	void fromJson(json_t* rootJ);

	/** Callback for processing the audio stream.
	`inputStride` and `outputStride` are the number of array elements between frames in the buffers.
	*/
	virtual void processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) {}
	/** Called before processBuffer() is called for all Ports of the same device.
	Splitting the processBuffer() into these calls is useful for synchronizing Ports of the same device.
	Called even if there are no inputs.
	*/
	virtual void processInput(const float* input, int inputStride, int frames) {}
	/** Called after processBuffer() is called for all Ports of the same device.
	*/
	virtual void processOutput(float* output, int outputStride, int frames) {}
	virtual void onOpenStream() {}
	virtual void onCloseStream() {}
};


void init();
void destroy();
/** Registers a new audio driver. Takes pointer ownership. */
void addDriver(int driverId, Driver* driver);
std::vector<int> getDriverIds();
Driver* getDriver(int driverId);


} // namespace audio
} // namespace rack
