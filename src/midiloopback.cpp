#include <midiloopback.hpp>
#include <context.hpp>
#include <string.hpp>


namespace rack {
namespace midiloopback {


static const int DRIVER_ID = -12;
static const size_t NUM_DEVICES = 16;


struct Device : midi::InputDevice, midi::OutputDevice {
	int id = -1;

	std::string getName() override {
		return string::f("Loopback %d", id + 1);
	}

	void sendMessage(const midi::Message& message) override {
		onMessage(message);
	}
};


struct Driver : midi::Driver {
	std::string getName() override {
		return "Loopback";
	}

	// Input methods
	std::vector<int> getInputDeviceIds() override {
		std::vector<int> deviceIds;
		for (size_t i = 0; i < NUM_DEVICES; i++) {
			deviceIds.push_back(i);
		}
		return deviceIds;
	}
	int getDefaultInputDeviceId() override {
		return 0;
	}
	std::string getInputDeviceName(int deviceId) override {
		return getDevice(deviceId)->getName();
	}
	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		midi::InputDevice* inputDevice = getDevice(deviceId);
		if (!inputDevice)
			return NULL;
		inputDevice->subscribe(input);
		return inputDevice;
	}
	void unsubscribeInput(int deviceId, midi::Input* input) override {
		midi::InputDevice* inputDevice = getDevice(deviceId);
		if (!inputDevice)
			return;
		inputDevice->unsubscribe(input);
	}

	// Output methods
	std::vector<int> getOutputDeviceIds() override {
		// Output IDs match input IDs
		return getInputDeviceIds();
	}
	int getDefaultOutputDeviceId() override {
		return getDefaultInputDeviceId();
	}
	std::string getOutputDeviceName(int deviceId) override {
		return getInputDeviceName(deviceId);
	}
	midi::OutputDevice* subscribeOutput(int deviceId, midi::Output* output) override {
		midi::OutputDevice* outputDevice = getDevice(deviceId);
		if (!outputDevice)
			return NULL;
		outputDevice->subscribe(output);
		return outputDevice;
	}
	void unsubscribeOutput(int deviceId, midi::Output* output) override {
		midi::OutputDevice* outputDevice = getDevice(deviceId);
		if (!outputDevice)
			return;
		outputDevice->unsubscribe(output);
	}

	// Custom methods
	Device* getDevice(int deviceId) {
		if (!APP->midiLoopbackContext)
			return NULL;
		if (!(0 <= deviceId && (size_t) deviceId < NUM_DEVICES))
			return NULL;
		Context* context = APP->midiLoopbackContext;
		return context->devices[deviceId];
	}
};


Context::Context() {
	for (size_t i = 0; i < NUM_DEVICES; i++) {
		Device* device = new Device;
		device->id = i;
		devices.push_back(device);
	}
}

Context::~Context() {
	for (Device* device : devices) {
		delete device;
	}
	devices.clear();
}


void init() {
	Driver* driver = new Driver;
	midi::addDriver(DRIVER_ID, driver);
}


} // namespace midiloopback
} // namespace rack
