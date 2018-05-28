#include "midi.hpp"
#include "rtmidi.hpp"
#include "bridge.hpp"
#include "gamepad.hpp"
#include "keyboard.hpp"


namespace rack {


static std::vector<int> driverIds;
static std::map<int, MidiDriver*> drivers;


////////////////////
// MidiDevice
////////////////////

void MidiInputDevice::subscribe(MidiInput *midiInput) {
	subscribed.insert(midiInput);
}

void MidiInputDevice::unsubscribe(MidiInput *midiInput) {
	// Remove MidiInput from subscriptions
	auto it = subscribed.find(midiInput);
	if (it != subscribed.end())
		subscribed.erase(it);
}

void MidiInputDevice::onMessage(MidiMessage message) {
	for (MidiInput *midiInput : subscribed) {
		midiInput->onMessage(message);
	}
}

////////////////////
// MidiDriver
////////////////////


////////////////////
// MidiIO
////////////////////

MidiIO::~MidiIO() {
	// Because of polymorphic destruction, descendants must call this in their own destructor
	// setDriverId(-1);
}

std::vector<int> MidiIO::getDriverIds() {
	return driverIds;
}

std::string MidiIO::getDriverName(int driverId) {
	auto it = drivers.find(driverId);
	if (it == drivers.end())
		return "";

	return it->second->getName();
}

void MidiIO::setDriverId(int driverId) {
	// Destroy driver
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

std::string MidiIO::getChannelName(int channel) {
	if (channel == -1)
		return "All channels";
	else
		return stringf("Channel %d", channel + 1);
}

json_t *MidiIO::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(driverId));
	std::string deviceName = getDeviceName(deviceId);
	if (!deviceName.empty())
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void MidiIO::fromJson(json_t *rootJ) {
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
// MidiInput
////////////////////

MidiInput::MidiInput() {
	if (driverIds.size() >= 1) {
		setDriverId(driverIds[0]);
	}
}

MidiInput::~MidiInput() {
	setDriverId(-1);
}

std::vector<int> MidiInput::getDeviceIds() {
	if (driver) {
		return driver->getInputDeviceIds();
	}
	return {};
}

std::string MidiInput::getDeviceName(int deviceId) {
	if (driver) {
		return driver->getInputDeviceName(deviceId);
	}
	return "";
}

void MidiInput::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		driver->unsubscribeInputDevice(this->deviceId, this);
	}
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		driver->subscribeInputDevice(deviceId, this);
		this->deviceId = deviceId;
	}
}

void MidiInputQueue::onMessage(MidiMessage message) {
	// Filter channel
	if (channel >= 0) {
		if (message.status() != 0xf && message.channel() != channel)
			return;
	}

	// Push to queue
	if ((int) queue.size() < queueMaxSize)
		queue.push(message);
}

bool MidiInputQueue::shift(MidiMessage *message) {
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
// MidiOutput
////////////////////

MidiOutput::MidiOutput() {
}

MidiOutput::~MidiOutput() {
	setDriverId(-1);
}

void MidiOutput::setDeviceId(int deviceId) {
	// TODO
}

////////////////////
// midi
////////////////////

void midiDestroy() {
	driverIds.clear();
	for (auto &pair : drivers) {
		delete pair.second;
	}
	drivers.clear();
}

void midiDriverAdd(int driverId, MidiDriver *driver) {
	assert(driver);
	driverIds.push_back(driverId);
	drivers[driverId] = driver;
}


} // namespace rack
