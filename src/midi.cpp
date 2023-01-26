#include <map>
#include <utility>
#include <queue>
#include <mutex>

#include <midi.hpp>
#include <string.hpp>
#include <system.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>


namespace rack {
namespace midi {


static std::vector<std::pair<int, Driver*>> drivers;

std::string Message::toString() const {
	std::string s;
	for (size_t i = 0; i < bytes.size(); i++) {
		if (i > 0)
			s += " ";
		uint8_t b = bytes[i];
		// We could use string::f() here, but use faster method instead.
		// s += string::f("%02x", b);
		uint8_t b1 = (b & 0x0f) >> 0;
		uint8_t b2 = (b & 0xf0) >> 4;
		s += b2 < 0xa ? ('0' + b2) : ('a' + b2 - 0xa);
		s += b1 < 0xa ? ('0' + b1) : ('a' + b1 - 0xa);
	}
	return s;
}


////////////////////
// Device
////////////////////

void InputDevice::subscribe(Input* input) {
	subscribed.insert(input);
}

void InputDevice::unsubscribe(Input* input) {
	// Remove Input from subscriptions
	auto it = subscribed.find(input);
	if (it != subscribed.end())
		subscribed.erase(it);
}

void InputDevice::onMessage(const Message& message) {
	for (Input* input : subscribed) {
		// Filter channel if message is not a system MIDI message
		if (message.getStatus() != 0xf && input->channel >= 0 && message.getChannel() != input->channel)
			continue;

		// We're probably in the MIDI driver's thread, so set the Rack context.
		contextSet(input->context);

		// Set timestamp to now if unset
		if (message.getFrame() < 0) {
			Message msg = message;
			double deltaTime = system::getTime() - APP->engine->getBlockTime();
			int64_t deltaFrames = std::floor(deltaTime * APP->engine->getSampleRate());
			// Delay message by current Engine block size
			deltaFrames += APP->engine->getBlockFrames();
			msg.setFrame(APP->engine->getBlockFrame() + deltaFrames);
			// Pass message to Input port
			input->onMessage(msg);
		}
		else {
			// Pass message to Input port
			input->onMessage(message);
		}
	}
}

void OutputDevice::subscribe(Output* output) {
	subscribed.insert(output);
}

void OutputDevice::unsubscribe(Output* output) {
	auto it = subscribed.find(output);
	if (it != subscribed.end())
		subscribed.erase(it);
}

////////////////////
// Port
////////////////////

Port::Port() {
	context = contextGet();
}

Port::~Port() {
}

Driver* Port::getDriver() {
	return driver;
}

int Port::getDriverId() {
	return driverId;
}

void Port::setDriverId(int driverId) {
	// Unset device and driver
	setDeviceId(-1);
	driver = NULL;
	this->driverId = -1;

	// Find driver by ID
	driver = midi::getDriver(driverId);
	if (driver) {
		this->driverId = driverId;
	}
	else if (!drivers.empty()) {
		// Set first driver as default
		driver = drivers[0].second;
		this->driverId = drivers[0].first;
	}
	else {
		// No fallback drivers
		return;
	}

	// Set default device if exists
	int defaultDeviceId = getDefaultDeviceId();
	if (defaultDeviceId >= 0)
		setDeviceId(defaultDeviceId);
}

Device* Port::getDevice() {
	return device;
}

int Port::getDeviceId() {
	return deviceId;
}

int Port::getChannel() {
	return channel;
}

void Port::setChannel(int channel) {
	this->channel = channel;
}

std::string Port::getChannelName(int channel) {
	if (channel < 0)
		return "All channels";
	else
		return string::f("Channel %d", channel + 1);
}

json_t* Port::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(getDriverId()));

	if (device) {
		std::string deviceName = device->getName();
		if (!deviceName.empty())
			json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	}

	json_object_set_new(rootJ, "channel", json_integer(getChannel()));
	return rootJ;
}

void Port::fromJson(json_t* rootJ) {
	setDriverId(-1);

	json_t* driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriverId(json_integer_value(driverJ));

	if (driver) {
		json_t* deviceNameJ = json_object_get(rootJ, "deviceName");
		if (deviceNameJ) {
			std::string deviceName = json_string_value(deviceNameJ);
			// Search for device ID with equal name
			for (int deviceId : getDeviceIds()) {
				if (getDeviceName(deviceId) == deviceName) {
					setDeviceId(deviceId);
					break;
				}
			}
		}
	}

	json_t* channelJ = json_object_get(rootJ, "channel");
	if (channelJ)
		channel = json_integer_value(channelJ);
}

////////////////////
// Input
////////////////////

Input::Input() {
	reset();
}

Input::~Input() {
	setDeviceId(-1);
}

void Input::reset() {
	setDriverId(-1);
	channel = -1;
}

std::vector<int> Input::getDeviceIds() {
	if (!driver)
		return {};
	try {
		return driver->getInputDeviceIds();
	}
	catch (Exception& e) {
		WARN("MIDI port could not get input device IDs: %s", e.what());
		return {};
	}
}

int Input::getDefaultDeviceId() {
	if (!driver)
		return -1;
	try {
		return driver->getDefaultInputDeviceId();
	}
	catch (Exception& e) {
		WARN("MIDI port get default input device ID: %s", e.what());
		return -1;
	}
}

void Input::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		try {
			driver->unsubscribeInput(this->deviceId, this);
		}
		catch (Exception& e) {
			WARN("MIDI port could not unsubscribe from input: %s", e.what());
		}
	}
	device = inputDevice = NULL;
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		try {
			device = inputDevice = driver->subscribeInput(deviceId, this);
			if (device) {
				this->deviceId = deviceId;
			}
		}
		catch (Exception& e) {
			WARN("MIDI port could not subscribe to input: %s", e.what());
		}
	}
}

std::string Input::getDeviceName(int deviceId) {
	if (!driver)
		return "";
	try {
		return driver->getInputDeviceName(deviceId);
	}
	catch (Exception& e) {
		WARN("MIDI port could not get input device name: %s", e.what());
		return "";
	}
}

std::vector<int> Input::getChannels() {
	std::vector<int> channels;
	for (int c = -1; c < 16; c++) {
		channels.push_back(c);
	}
	return channels;
}

////////////////////
// InputQueue
////////////////////

static const size_t InputQueue_maxSize = 8192;

struct InputQueue_Compare {
	bool operator()(const Message& a, const Message& b) {
		return a.getFrame() > b.getFrame();
	}
};

struct InputQueue_Queue : std::priority_queue<Message, std::vector<Message>, InputQueue_Compare> {
	void reserve(size_t capacity) {
		c.reserve(capacity);
	}
	void clear() {
		// Messing with the protected container is dangerous, but completely clearing it should be fine.
		c.clear();
	}
};

struct InputQueue::Internal {
	InputQueue_Queue queue;
	std::mutex mutex;
};

InputQueue::InputQueue() {
	internal = new Internal;
	internal->queue.reserve(InputQueue_maxSize);
}

InputQueue::~InputQueue() {
	delete internal;
}

void InputQueue::onMessage(const Message& message) {
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Reject MIDI message if queue is full
	if (internal->queue.size() >= InputQueue_maxSize)
		return;
	// Push to queue
	internal->queue.push(message);
}

bool InputQueue::tryPop(Message* messageOut, int64_t maxFrame) {
	if (internal->queue.empty())
		return false;

	std::lock_guard<std::mutex> lock(internal->mutex);
	const Message& msg = internal->queue.top();
	if (msg.getFrame() <= maxFrame) {
		*messageOut = msg;
		internal->queue.pop();
		return true;
	}

	return false;
}

size_t InputQueue::size() {
	return internal->queue.size();
}


////////////////////
// Output
////////////////////

Output::Output() {
	reset();
}

Output::~Output() {
	setDeviceId(-1);
}

void Output::reset() {
	setDriverId(-1);
	channel = 0;
}

std::vector<int> Output::getDeviceIds() {
	if (!driver)
		return {};
	try {
		return driver->getOutputDeviceIds();
	}
	catch (Exception& e) {
		WARN("MIDI port could not get output device IDs: %s", e.what());
		return {};
	}
}

void Output::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		try {
			driver->unsubscribeOutput(this->deviceId, this);
		}
		catch (Exception& e) {
			WARN("MIDI port could not unsubscribe from output: %s", e.what());
		}
	}
	device = outputDevice = NULL;
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		try {
			device = outputDevice = driver->subscribeOutput(deviceId, this);
			if (device) {
				this->deviceId = deviceId;
			}
		}
		catch (Exception& e) {
			WARN("MIDI port could not subscribe to output: %s", e.what());
		}
	}
}

int Output::getDefaultDeviceId() {
	if (!driver)
		return -1;
	try {
		return driver->getDefaultOutputDeviceId();
	}
	catch (Exception& e) {
		WARN("MIDI port get default output device ID: %s", e.what());
		return -1;
	}
}

std::string Output::getDeviceName(int deviceId) {
	if (!driver)
		return "";
	try {
		return driver->getOutputDeviceName(deviceId);
	}
	catch (Exception& e) {
		WARN("MIDI port could not get output device name: %s", e.what());
		return "";
	}
}

std::vector<int> Output::getChannels() {
	std::vector<int> channels;
	for (int c = 0; c < 16; c++) {
		channels.push_back(c);
	}
	return channels;
}

void Output::sendMessage(const Message& message) {
	if (!outputDevice)
		return;

	// Set channel if message is not a system MIDI message
	Message msg = message;
	if (msg.getStatus() != 0xf && channel >= 0) {
		msg.setChannel(channel);
	}
	// DEBUG("sendMessage %02x %02x %02x", msg.cmd, msg.data1, msg.data2);
	try {
		outputDevice->sendMessage(msg);
	}
	catch (Exception& e) {
		// Don't log error because it could flood the log.
		// WARN("MIDI port could not be sent MIDI message: %s", e.what());
		// TODO Perhaps `setDevice(-1)` if sending message fails?
	}
}


////////////////////
// midi
////////////////////

void init() {
}

void destroy() {
	for (auto& pair : drivers) {
		delete pair.second;
	}
	drivers.clear();
}

void addDriver(int driverId, Driver* driver) {
	assert(driver);
	drivers.push_back(std::make_pair(driverId, driver));
}

std::vector<int> getDriverIds() {
	std::vector<int> driverIds;
	for (auto& pair : drivers) {
		driverIds.push_back(pair.first);
	}
	return driverIds;
}

Driver* getDriver(int driverId) {
	// Search for driver by ID
	for (auto& pair : drivers) {
		if (pair.first == driverId)
			return pair.second;
	}
	return NULL;
}


} // namespace midi
} // namespace rack
