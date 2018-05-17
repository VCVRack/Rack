#include "midi.hpp"
#include "rtmidi.hpp"
#include "bridge.hpp"
#include "gamepad.hpp"


namespace rack {


////////////////////
// MidiIODevice
////////////////////

void MidiInputDevice::subscribe(MidiInput *midiInput) {
	subscribed.insert(midiInput);
}

void MidiInputDevice::unsubscribe(MidiInput *midiInput) {
	auto it = subscribed.find(midiInput);
	if (it != subscribed.end())
		subscribed.erase(it);

	if (subscribed.size() == 0) {
		warn("TODO: Fix memory leak");
	}
}

void MidiInputDevice::onMessage(MidiMessage message) {
	for (MidiInput *midiInput : subscribed) {
		midiInput->onMessage(message);
	}
}

////////////////////
// MidiIODriver
////////////////////


////////////////////
// MidiIO
////////////////////

std::vector<int> MidiIO::getDriverIds() {
	std::vector<int> driverIds = rtmidiGetDrivers();
	// Add custom driverIds
	driverIds.push_back(BRIDGE_DRIVER);
	driverIds.push_back(GAMEPAD_DRIVER);
	return driverIds;
}

std::string MidiIO::getDriverName(int driverId) {
	switch (driverId) {
		case RtMidi::UNSPECIFIED: return "Unspecified";
		case RtMidi::MACOSX_CORE: return "Core MIDI";
		case RtMidi::LINUX_ALSA: return "ALSA";
		case RtMidi::UNIX_JACK: return "JACK";
		case RtMidi::WINDOWS_MM: return "Windows MIDI";
		case RtMidi::RTMIDI_DUMMY: return "Dummy MIDI";
		case BRIDGE_DRIVER: return "Bridge";
		case GAMEPAD_DRIVER: return "Gamepad";
		default: return "Unknown";
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
}

MidiInput::~MidiInput() {
	setDriverId(-1);
}

void MidiInput::setDriverId(int driverId) {
	// Destroy driver
	setDeviceId(-1);
	if (driver) {
		driver = NULL;
	}
	this->driverId = -1;

	// Create driver
	if (driverId >= 0) {
		driver = rtmidiGetInputDriver(driverId);
	}
	else if (driverId == BRIDGE_DRIVER) {
		// TODO
	}
	else if (driverId == GAMEPAD_DRIVER) {
		driver = gamepadGetInputDriver();
	}

	// Set driverId
	if (driver) {
		this->driverId = driverId;
	}
}

std::vector<int> MidiInput::getDeviceIds() {
	if (driver) {
		return driver->getDeviceIds();
	}
	return {};
}

std::string MidiInput::getDeviceName(int deviceId) {
	if (driver) {
		return driver->getDeviceName(deviceId);
	}
	return "";
}

void MidiInput::setDeviceId(int deviceId) {
	// Destroy device
	if (device) {
		device->unsubscribe(this);
		device = NULL;
	}
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		device = driver->getDevice(deviceId);
		device->subscribe(this);
	}

	// Set deviceId
	if (device) {
		this->deviceId = deviceId;
	}
}

void MidiInputQueue::onMessage(MidiMessage message) {
	// Filter channel
	if (channel >= 0) {
		if (message.status() != 0xf && message.channel() != channel)
			return;
	}

	if ((int) queue.size() < queueSize)
		queue.push(message);
}

bool MidiInputQueue::shift(MidiMessage *message) {
	if (!message) return false;
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
	// TODO
}

void MidiOutput::setDriverId(int driverId) {
	// TODO
}

void MidiOutput::setDeviceId(int deviceId) {
	// TODO
}


} // namespace rack
