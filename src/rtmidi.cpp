#include <rtmidi.hpp>
#include <midi.hpp>
#include <map>

#pragma GCC diagnostic push
#ifndef __clang__
	#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <rtmidi/RtMidi.h>
#pragma GCC diagnostic pop


namespace rack {


struct RtMidiInputDevice : midi::InputDevice {
	RtMidiIn* rtMidiIn;
	std::string name;

	RtMidiInputDevice(int driverId, int deviceId) {
		rtMidiIn = new RtMidiIn((RtMidi::Api) driverId, "VCV Rack");
		assert(rtMidiIn);
		rtMidiIn->ignoreTypes(false, false, false);
		rtMidiIn->setCallback(midiInputCallback, this);
		name = rtMidiIn->getPortName(deviceId);
		rtMidiIn->openPort(deviceId, "VCV Rack input");
	}

	~RtMidiInputDevice() {
		rtMidiIn->closePort();
		delete rtMidiIn;
	}

	std::string getName() override {
		return name;
	}

	static void midiInputCallback(double timeStamp, std::vector<unsigned char>* message, void* userData) {
		if (!message)
			return;
		if (!userData)
			return;

		RtMidiInputDevice* midiInputDevice = (RtMidiInputDevice*) userData;
		if (!midiInputDevice)
			return;

		midi::Message msg;
		msg.bytes = std::vector<uint8_t>(message->begin(), message->end());
		midiInputDevice->onMessage(msg);
	}
};


struct RtMidiOutputDevice : midi::OutputDevice {
	RtMidiOut* rtMidiOut;
	std::string name;

	RtMidiOutputDevice(int driverId, int deviceId) {
		rtMidiOut = new RtMidiOut((RtMidi::Api) driverId, "VCV Rack");
		assert(rtMidiOut);
		name = rtMidiOut->getPortName(deviceId);
		rtMidiOut->openPort(deviceId, "VCV Rack output");
	}

	~RtMidiOutputDevice() {
		rtMidiOut->closePort();
		delete rtMidiOut;
	}

	std::string getName() override {
		return name;
	}

	void sendMessage(const midi::Message &message) override {
		std::vector<unsigned char> bytes(message.bytes.begin(), message.bytes.end());
		rtMidiOut->sendMessage(&bytes);
	}
};


struct RtMidiDriver : midi::Driver {
	int driverId;
	/** Just for querying MIDI driver information */
	RtMidiIn* rtMidiIn;
	RtMidiOut* rtMidiOut;
	std::map<int, RtMidiInputDevice*> inputDevices;
	std::map<int, RtMidiOutputDevice*> outputDevices;

	RtMidiDriver(int driverId) {
		this->driverId = driverId;
		rtMidiIn = new RtMidiIn((RtMidi::Api) driverId);
		assert(rtMidiIn);
		rtMidiOut = new RtMidiOut((RtMidi::Api) driverId);
		assert(rtMidiOut);
	}

	~RtMidiDriver() {
		assert(inputDevices.empty());
		assert(outputDevices.empty());
		delete rtMidiIn;
		delete rtMidiOut;
	}

	std::string getName() override {
		switch (driverId) {
			case RtMidi::UNSPECIFIED: return "Unspecified";
			case RtMidi::MACOSX_CORE: return "Core MIDI";
			case RtMidi::LINUX_ALSA: return "ALSA";
			case RtMidi::UNIX_JACK: return "JACK";
			case RtMidi::WINDOWS_MM: return "Windows MIDI";
			case RtMidi::RTMIDI_DUMMY: return "Dummy MIDI";
			default: return "";
		}
	}
	std::vector<int> getInputDeviceIds() override {
		// TODO The IDs unfortunately jump around in RtMidi. Is there a way to keep them constant when a MIDI device is added/removed?
		int count = rtMidiIn->getPortCount();
		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getInputDeviceName(int deviceId) override {
		if (deviceId >= 0) {
			return rtMidiIn->getPortName(deviceId);
		}
		return "";
	}

	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		if (!(0 <= deviceId && deviceId < (int) rtMidiIn->getPortCount()))
			return NULL;
		RtMidiInputDevice* device = inputDevices[deviceId];
		if (!device) {
			inputDevices[deviceId] = device = new RtMidiInputDevice(driverId, deviceId);
		}

		device->subscribe(input);
		return device;
	}

	void unsubscribeInput(int deviceId, midi::Input* input) override {
		auto it = inputDevices.find(deviceId);
		if (it == inputDevices.end())
			return;
		RtMidiInputDevice* device = it->second;
		device->unsubscribe(input);

		// Destroy device if nothing is subscribed anymore
		if (device->subscribed.empty()) {
			inputDevices.erase(it);
			delete device;
		}
	}

	std::vector<int> getOutputDeviceIds() override {
		int count = rtMidiOut->getPortCount();
		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getOutputDeviceName(int deviceId) override {
		if (deviceId >= 0) {
			return rtMidiOut->getPortName(deviceId);
		}
		return "";
	}

	midi::OutputDevice* subscribeOutput(int deviceId, midi::Output* output) override {
		if (!(0 <= deviceId && deviceId < (int) rtMidiOut->getPortCount()))
			return NULL;
		RtMidiOutputDevice* device = outputDevices[deviceId];
		if (!device) {
			outputDevices[deviceId] = device = new RtMidiOutputDevice(driverId, deviceId);
		}

		device->subscribe(output);
		return device;
	}

	void unsubscribeOutput(int deviceId, midi::Output* output) override {
		auto it = outputDevices.find(deviceId);
		if (it == outputDevices.end())
			return;
		RtMidiOutputDevice* device = it->second;
		device->unsubscribe(output);

		// Destroy device if nothing is subscribed anymore
		if (device->subscribed.empty()) {
			outputDevices.erase(it);
			delete device;
		}
	}
};


void rtmidiInit() {
	std::vector<RtMidi::Api> rtApis;
	RtMidi::getCompiledApi(rtApis);
	for (RtMidi::Api api : rtApis) {
		int driverId = (int) api;
		midi::Driver* driver = new RtMidiDriver(driverId);
		midi::addDriver(driverId, driver);
	}
}


} // namespace rack
