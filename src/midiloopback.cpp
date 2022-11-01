#include <context.hpp>
#include <midiloopback.hpp>


namespace rack {
namespace midiloopback {


static const int DRIVER_ID = -12;


struct Device : midi::InputDevice, midi::OutputDevice {
	std::string getName() override {
		return "Loopback";
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
		return {0};
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
		return {0};
	}
	int getDefaultOutputDeviceId() override {
		return 0;
	}
	std::string getOutputDeviceName(int deviceId) override {
		return getDevice(deviceId)->getName();
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
		if (deviceId != 0)
			return NULL;
		return APP->midiLoopbackContext->devices[deviceId];
	}
};


Context::Context() {
	devices[0] = new Device;
}

Context::~Context() {
	delete devices[0];
}


void init() {
	Driver* driver = new Driver;
	midi::addDriver(DRIVER_ID, driver);
}


} // namespace midiloopback
} // namespace rack
