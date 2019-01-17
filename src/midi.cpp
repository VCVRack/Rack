#include "midi.hpp"
#include "string.hpp"
#include <map>


namespace rack {
namespace midi {


/** Preserves the order of IDs */
static std::vector<int> driverIds;
static std::map<int, Driver*> drivers;


////////////////////
// Device
////////////////////

void InputDevice::subscribe(Input *input) {
	subscribed.insert(input);
}

void InputDevice::unsubscribe(Input *input) {
	// Remove Input from subscriptions
	auto it = subscribed.find(input);
	if (it != subscribed.end())
		subscribed.erase(it);
}

void InputDevice::onMessage(Message message) {
	for (Input *input : subscribed) {
		// Filter channel
		if (input->channel < 0 || message.status() == 0xf || message.channel() == input->channel) {
			input->onMessage(message);
		}
	}
}

void OutputDevice::subscribe(Output *output) {
	subscribed.insert(output);
}

void OutputDevice::unsubscribe(Output *output) {
	// Remove Output from subscriptions
	auto it = subscribed.find(output);
	if (it != subscribed.end())
		subscribed.erase(it);
}

////////////////////
// IO
////////////////////

std::vector<int> IO::getDriverIds() {
	return driverIds;
}

std::string IO::getDriverName(int driverId) {
	auto it = drivers.find(driverId);
	if (it == drivers.end())
		return "";

	return it->second->getName();
}

void IO::setDriverId(int driverId) {
	// Unset device and driver
	setDeviceId(-1);
	if (driver) {
		driver = NULL;
	}
	this->driverId = -1;

	// Set driver
	auto it = drivers.find(driverId);
	if (it != drivers.end()) {
		driver = it->second;
		this->driverId = driverId;
	}
}

std::string IO::getChannelName(int channel) {
	if (channel == -1)
		return "All channels";
	else
		return string::f("Channel %d", channel + 1);
}

void IO::setChannel(int channel) {
	this->channel = channel;
}

json_t *IO::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(driverId));
	std::string deviceName = getDeviceName(deviceId);
	if (!deviceName.empty())
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void IO::fromJson(json_t *rootJ) {
	json_t *driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriverId(json_integer_value(driverJ));

	json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device with equal name
		for (int deviceId : getDeviceIds()) {
			if (getDeviceName(deviceId) == deviceName) {
				setDeviceId(deviceId);
				break;
			}
		}
	}

	json_t *channelJ = json_object_get(rootJ, "channel");
	if (channelJ)
		channel = json_integer_value(channelJ);
}

////////////////////
// Input
////////////////////

Input::Input() {
	// Set first driver as default
	if (driverIds.size() >= 1) {
		setDriverId(driverIds[0]);
	}
}

Input::~Input() {
	setDriverId(-1);
}

std::vector<int> Input::getDeviceIds() {
	if (driver) {
		return driver->getInputDeviceIds();
	}
	return {};
}

std::string Input::getDeviceName(int deviceId) {
	if (driver) {
		return driver->getInputDeviceName(deviceId);
	}
	return "";
}

void Input::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		driver->unsubscribeInput(this->deviceId, this);
		inputDevice = NULL;
	}
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		inputDevice = driver->subscribeInput(deviceId, this);
		this->deviceId = deviceId;
	}
}

void InputQueue::onMessage(Message message) {
	// Push to queue
	if ((int) queue.size() < queueMaxSize)
		queue.push(message);
}

bool InputQueue::shift(Message *message) {
	if (!message)
		return false;
	if (!queue.empty()) {
		*message = queue.front();
		queue.pop();
		return true;
	}
	return false;
}

////////////////////
// Output
////////////////////

Output::Output() {
	// Set first driver as default
	if (driverIds.size() >= 1) {
		setDriverId(driverIds[0]);
	}
}

Output::~Output() {
	setDriverId(-1);
}

std::vector<int> Output::getDeviceIds() {
	if (driver) {
		return driver->getOutputDeviceIds();
	}
	return {};
}

std::string Output::getDeviceName(int deviceId) {
	if (driver) {
		return driver->getOutputDeviceName(deviceId);
	}
	return "";
}

void Output::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		driver->unsubscribeOutput(this->deviceId, this);
		outputDevice = NULL;
	}
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		outputDevice = driver->subscribeOutput(deviceId, this);
		this->deviceId = deviceId;
	}
}

void Output::sendMessage(Message message) {
	if (outputDevice) {
		outputDevice->sendMessage(message);
	}
}


////////////////////
// midi
////////////////////

void init() {
}

void destroy() {
	driverIds.clear();
	for (auto &pair : drivers) {
		delete pair.second;
	}
	drivers.clear();
}

void addDriver(int driverId, Driver *driver) {
	assert(driver);
	driverIds.push_back(driverId);
	drivers[driverId] = driver;
}


} // namespace midi
} // namespace rack
