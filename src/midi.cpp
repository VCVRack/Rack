#include <midi.hpp>
#include <string.hpp>
#include <map>
#include <utility>


namespace rack {
namespace midi {


static std::vector<std::pair<int, Driver*>> drivers;


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

void InputDevice::onMessage(Message message) {
	for (Input* input : subscribed) {
		// Filter channel
		if (input->channel < 0 || message.getStatus() == 0xf || message.getChannel() == input->channel) {
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

std::vector<int> Port::getDriverIds() {
	std::vector<int> driverIds;
	for (auto& pair : drivers) {
		driverIds.push_back(pair.first);
	}
	return driverIds;
}

std::string Port::getDriverName(int driverId) {
	for (auto& pair : drivers) {
		if (pair.first == driverId) {
			return pair.second->getName();
		}
	}
	return "";
}

void Port::setDriverId(int driverId) {
	// Unset device and driver
	setDeviceId(-1);
	driver = NULL;
	this->driverId = -1;

	// Set driver
	for (auto& pair : drivers) {
		if (pair.first == driverId) {
			driver = pair.second;
			this->driverId = driverId;
			break;
		}
	}
}

std::string Port::getChannelName(int channel) {
	if (channel == -1)
		return "All channels";
	else
		return string::f("Channel %d", channel + 1);
}

void Port::setChannel(int channel) {
	this->channel = channel;
}

json_t* Port::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(driverId));
	std::string deviceName = getDeviceName(deviceId);
	if (!deviceName.empty())
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void Port::fromJson(json_t* rootJ) {
	json_t* driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriverId(json_integer_value(driverJ));

	json_t* deviceNameJ = json_object_get(rootJ, "deviceName");
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
	setDriverId(-1);
}

void Input::reset() {
	channel = -1;
	// Set first driver as default
	if (drivers.size() >= 1) {
		setDriverId(drivers[0].first);
	}
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
	}
	inputDevice = NULL;
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		inputDevice = driver->subscribeInput(deviceId, this);
		this->deviceId = deviceId;
	}
}

std::vector<int> Input::getChannels() {
	std::vector<int> channels;
	for (int c = -1; c < 16; c++) {
		channels.push_back(c);
	}
	return channels;
}

void InputQueue::onMessage(Message message) {
	// Push to queue
	if ((int) queue.size() < queueMaxSize)
		queue.push(message);
}

bool InputQueue::shift(Message* message) {
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
	reset();
}

Output::~Output() {
	setDriverId(-1);
}

void Output::reset() {
	channel = 0;
	// Set first driver as default
	if (drivers.size() >= 1) {
		setDriverId(drivers[0].first);
	}
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
	}
	outputDevice = NULL;
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		outputDevice = driver->subscribeOutput(deviceId, this);
		this->deviceId = deviceId;
	}
}

std::vector<int> Output::getChannels() {
	std::vector<int> channels;
	for (int c = 0; c < 16; c++) {
		channels.push_back(c);
	}
	return channels;
}

void Output::sendMessage(Message message) {
	// Set channel
	if (message.getStatus() != 0xf) {
		message.setChannel(channel);
	}
	// DEBUG("sendMessage %02x %02x %02x", message.cmd, message.data1, message.data2);
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
	for (auto& pair : drivers) {
		delete pair.second;
	}
	drivers.clear();
}

void addDriver(int driverId, Driver* driver) {
	assert(driver);
	drivers.push_back(std::make_pair(driverId, driver));
}


} // namespace midi
} // namespace rack
