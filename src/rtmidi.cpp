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
#include <string.hpp>
#include <system.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>


namespace rack {


static void rtMidiErrorCallback(RtMidiError::Type type, const std::string& errorText, void* userData) {
	// Do nothing
}


struct RtMidiInputDevice : midi::InputDevice {
	RtMidiIn* rtMidiIn;
	std::string name;

	RtMidiInputDevice(int driverId, int deviceId) {
		try {
			rtMidiIn = new RtMidiIn((RtMidi::Api) driverId, "VCV Rack");
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to create RtMidi input driver %d: %s", driverId, e.what());
		}
		rtMidiIn->setErrorCallback(rtMidiErrorCallback);
		rtMidiIn->ignoreTypes(false, false, false);
		rtMidiIn->setCallback(midiInputCallback, this);

		try {
			name = rtMidiIn->getPortName(deviceId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi input device name: %s", e.what());
		}

		try {
			rtMidiIn->openPort(deviceId, "VCV Rack input");
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to open RtMidi input device: %s", e.what());
		}
	}

	~RtMidiInputDevice() {
		// This does not throw for any driver API
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

		RtMidiInputDevice* that = (RtMidiInputDevice*) userData;
		if (!that)
			return;

		midi::Message msg;
		msg.bytes = std::vector<uint8_t>(message->begin(), message->end());
		// Don't set msg.frame from timeStamp here, because it's set in onMessage().
		that->onMessage(msg);
	}
};


struct RtMidiOutputDevice : midi::OutputDevice {
	RtMidiOut* rtMidiOut;
	std::string name;

	struct MessageSchedule {
		midi::Message message;
		double timestamp;

		bool operator<(const MessageSchedule& other) const {
			return timestamp > other.timestamp;
		}
	};
	std::priority_queue<MessageSchedule, std::vector<MessageSchedule>> messageQueue;

	std::thread thread;
	std::mutex mutex;
	std::condition_variable cv;
	bool stopped = false;

	RtMidiOutputDevice(int driverId, int deviceId) {
		try {
			rtMidiOut = new RtMidiOut((RtMidi::Api) driverId, "VCV Rack");
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to create RtMidi output driver %d: %s", driverId, e.what());
		}
		rtMidiOut->setErrorCallback(rtMidiErrorCallback);

		try {
			name = rtMidiOut->getPortName(deviceId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi output device name: %s", e.what());
		}

		try {
			rtMidiOut->openPort(deviceId, "VCV Rack output");
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi output device name: %s", e.what());
		}

		startThread();
	}

	~RtMidiOutputDevice() {
		stopThread();
		// This does not throw for any driver API
		rtMidiOut->closePort();
		delete rtMidiOut;
	}

	std::string getName() override {
		return name;
	}

	void sendMessage(const midi::Message& message) override {
		// If frame is undefined, send message immediately
		if (message.getFrame() < 0) {
			sendMessageNow(message);
			return;
		}
		// Schedule message to be sent by worker thread
		MessageSchedule ms;
		ms.message = message;
		int64_t deltaFrames = message.getFrame() - APP->engine->getBlockFrame();
		// Delay message by current Engine block size
		deltaFrames += APP->engine->getBlockFrames();
		// Compute time in next Engine block to send message
		double deltaTime = deltaFrames * APP->engine->getSampleTime();
		ms.timestamp = APP->engine->getBlockTime() + deltaTime;

		std::lock_guard<decltype(mutex)> lock(mutex);
		messageQueue.push(ms);
		cv.notify_one();
	}

	// Consumer thread methods

	void startThread() {
		thread = std::thread(&RtMidiOutputDevice::runThread, this);
	}

	void runThread() {
		system::setThreadName("RtMidi output");

		std::unique_lock<decltype(mutex)> lock(mutex);
		while (!stopped) {
			if (messageQueue.empty()) {
				// No messages. Wait on the CV to be notified.
				cv.wait(lock);
			}
			else {
				// Get earliest message
				const MessageSchedule& ms = messageQueue.top();
				double duration = ms.timestamp - system::getTime();

				// If we need to wait, release the lock and wait for the timeout, or if the CV is notified.
				// This correctly handles MIDI messages with no timestamp, because duration will be NAN.
				if (duration > 0) {
					if (cv.wait_for(lock, std::chrono::duration<double>(duration)) != std::cv_status::timeout)
						continue;
				}

				// Send and remove from queue
				sendMessageNow(ms.message);
				messageQueue.pop();
			}
		}
	}

	void sendMessageNow(const midi::Message& message) {
		try {
			rtMidiOut->sendMessage(message.bytes.data(), message.bytes.size());
		}
		catch (RtMidiError& e) {
			// Ignore error
		}
	}

	void stopThread() {
		{
			std::lock_guard<decltype(mutex)> lock(mutex);
			stopped = true;
			cv.notify_one();
		}
		if (thread.joinable())
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

		try {
			rtMidiIn = new RtMidiIn((RtMidi::Api) driverId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to create RtMidi input driver %d: %s", driverId, e.what());
		}
		rtMidiIn->setErrorCallback(rtMidiErrorCallback);

		try {
			rtMidiOut = new RtMidiOut((RtMidi::Api) driverId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to create RtMidi output driver %d: %s", driverId, e.what());
		}
		rtMidiOut->setErrorCallback(rtMidiErrorCallback);
	}

	~RtMidiDriver() {
		assert(inputDevices.empty());
		assert(outputDevices.empty());
		// This does not throw for any driver API
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
		int count;
		try {
			count = rtMidiIn->getPortCount();
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi input device count: %s", e.what());
		}

		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getInputDeviceName(int deviceId) override {
		if (deviceId < 0)
			return "";
		try {
			return rtMidiIn->getPortName(deviceId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi input device name: %s", e.what());
		}
	}

	midi::InputDevice* subscribeInput(int deviceId, midi::Input* input) override {
		if (!(0 <= deviceId && deviceId < (int) rtMidiIn->getPortCount()))
			return NULL;
		RtMidiInputDevice* device = get(inputDevices, deviceId, NULL);
		if (!device) {
			try {
				inputDevices[deviceId] = device = new RtMidiInputDevice(driverId, deviceId);
			}
			catch (RtMidiError& e) {
				throw Exception("Failed to create RtMidi input device: %s", e.what());
			}
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
			try {
				delete device;
			}
			catch (RtMidiError& e) {
				throw Exception("Failed to delete RtMidi input device: %s", e.what());
			}
		}
	}

	std::vector<int> getOutputDeviceIds() override {
		// TODO The IDs unfortunately jump around in RtMidi. Is there a way to keep them constant when a MIDI device is added/removed?
		int count;
		try {
			count = rtMidiOut->getPortCount();
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi output device count: %s", e.what());
		}

		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getOutputDeviceName(int deviceId) override {
		if (deviceId < 0)
			return "";
		try {
			return rtMidiOut->getPortName(deviceId);
		}
		catch (RtMidiError& e) {
			throw Exception("Failed to get RtMidi output device count: %s", e.what());
		}
	}

	midi::OutputDevice* subscribeOutput(int deviceId, midi::Output* output) override {
		if (!(0 <= deviceId && deviceId < (int) rtMidiOut->getPortCount()))
			return NULL;
		RtMidiOutputDevice* device = get(outputDevices, deviceId, NULL);
		if (!device) {
			try {
				outputDevices[deviceId] = device = new RtMidiOutputDevice(driverId, deviceId);
			}
			catch (RtMidiError& e) {
				throw Exception("Failed to create RtMidi output device: %s", e.what());
			}
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
			try {
				delete device;
			}
			catch (RtMidiError& e) {
				throw Exception("Failed to delete RtMidi output device: %s", e.what());
			}
		}
	}
};


void rtmidiInit() {
	std::vector<RtMidi::Api> rtApis;
	RtMidi::getCompiledApi(rtApis);
	for (RtMidi::Api api : rtApis) {
		int driverId = (int) api;
		try {
			midi::Driver* driver = new RtMidiDriver(driverId);
			midi::addDriver(driverId, driver);
		}
		catch (Exception& e) {
			WARN("Could not create RtMidiDriver %d", api);
		}
	}
}


} // namespace rack
