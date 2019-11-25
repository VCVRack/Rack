#pragma once
#include <common.hpp>
#include <jansson.h>
#include <vector>
#include <set>

namespace rack {


/** Audio driver
*/
namespace audio {


////////////////////
// Driver
////////////////////

struct Port;
struct Device;

struct Driver {
	virtual ~Driver() {}
	virtual std::string getName() {
		return "";
	}
	virtual std::vector<int> getDeviceIds() {
		return {};
	}

	/** Gets the name of a device without subscribing to it. */
	virtual std::string getDeviceName(int deviceId) {
		return "";
	}
	virtual int getDeviceNumInputs(int deviceId) {
		return 0;
	}
	virtual int getDeviceNumOutputs(int deviceId) {
		return 0;
	}
	std::string getDeviceDetail(int deviceId, int offset, int maxChannels);

	virtual Device* subscribe(int deviceId, Port* port) {
		return NULL;
	}
	virtual void unsubscribe(int deviceId, Port* port) {}
};

////////////////////
// Device
////////////////////

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
	std::string getDetail(int offset, int maxChannels);

	virtual std::vector<int> getSampleRates() {
		return {};
	}
	virtual int getSampleRate() {
		return 0;
	}
	virtual void setSampleRate(int sampleRate) {}

	virtual std::vector<int> getBlockSizes() {
		return {};
	}
	virtual int getBlockSize() {
		return 0;
	}
	virtual void setBlockSize(int blockSize) {}

	// Called by this Device class, forwards to subscribed Ports.
	void processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames);
	void onOpenStream();
	void onCloseStream();
};

////////////////////
// Port
////////////////////

struct Port {
	int offset = 0;
	int maxChannels = 8;

	// private
	int driverId = -1;
	int deviceId = -1;
	/** Not owned */
	Driver* driver = NULL;
	Device* device = NULL;

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
	int getDeviceId() {
		return deviceId;
	}
	void setDeviceId(int deviceId);

	std::vector<int> getSampleRates() {
		if (!device)
			return {};
		return device->getSampleRates();
	}
	int getSampleRate() {
		if (!device)
			return 0;
		return device->getSampleRate();
	}
	void setSampleRate(int sampleRate) {
		if (device)
			device->setSampleRate(sampleRate);
	}

	std::vector<int> getBlockSizes() {
		if (!device)
			return {};
		return device->getBlockSizes();
	}
	int getBlockSize() {
		if (!device)
			return 0;
		return device->getBlockSize();
	}
	void setBlockSize(int blockSize) {
		if (device)
			device->setBlockSize(blockSize);
	}

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
