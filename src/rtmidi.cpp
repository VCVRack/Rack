#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#pragma GCC diagnostic push
#ifndef __clang__
	#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <rtmidi/RtMidi.h>
#pragma GCC diagnostic pop

#include <rtmidi.hpp>
#include <midi.hpp>
#include <system.hpp>


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


/** Makes priority_queue prioritize by the earliest MIDI message. */
static auto messageEarlier = [](const midi::Message& a, const midi::Message& b) {
	return a.timestamp > b.timestamp;
};


struct RtMidiOutputDevice : midi::OutputDevice {
	RtMidiOut* rtMidiOut;
	std::string name;

	std::priority_queue<midi::Message, std::vector<midi::Message>, decltype(messageEarlier)> messageQueue;

	std::thread thread;
	std::mutex mutex;
	std::condition_variable cv;
	bool stopped = false;

	RtMidiOutputDevice(int driverId, int deviceId) : messageQueue(messageEarlier) {
		rtMidiOut = new RtMidiOut((RtMidi::Api) driverId, "VCV Rack");
		assert(rtMidiOut);
		name = rtMidiOut->getPortName(deviceId);
		rtMidiOut->openPort(deviceId, "VCV Rack output");

		start();
	}

	~RtMidiOutputDevice() {
		stop();
		rtMidiOut->closePort();
		delete rtMidiOut;
	}

	std::string getName() override {
		return name;
	}

	void sendMessage(const midi::Message &message) override {
		// sendMessageNow(message);
		std::lock_guard<decltype(mutex)> lock(mutex);
		messageQueue.push(message);
		cv.notify_one();
	}

	// Consumer thread methods

	void start() {
		thread = std::thread(&RtMidiOutputDevice::run, this);
	}

	void run() {
		std::unique_lock<decltype(mutex)> lock(mutex);
		while (!stopped) {
			if (messageQueue.empty()) {
				// No messages. Wait on the CV to be notified.
				cv.wait(lock);
			}
			else {
				// Get earliest message
				const midi::Message& message = messageQueue.top();
				int64_t duration = message.timestamp - system::getNanoseconds();

				// If we need to wait, release the lock and wait for the timeout, or if the CV is notified.
				// This correctly handles MIDI messages with no timestamp, because duration will be negative.
				if ((duration > 0) && cv.wait_for(lock, std::chrono::nanoseconds(duration)) != std::cv_status::timeout)
					continue;

				// Send and remove from queue
				sendMessageNow(message);
				messageQueue.pop();
			}
		}
	}

	void sendMessageNow(const midi::Message &message) {
		try {
			rtMidiOut->sendMessage(message.bytes.data(), message.bytes.size());
		}
		catch (RtMidiError& e) {
			// Ignore error
		}
	}

	void stop() {
		{
			std::lock_guard<decltype(mutex)> lock(mutex);
			stopped = true;
			cv.notify_one();
		}
		thread.join();
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
