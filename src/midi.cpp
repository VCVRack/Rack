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

std::vector<int> MidiIO::getDrivers() {
	std::vector<int> drivers = rtmidiGetDrivers();
	// Add custom drivers
	drivers.push_back(BRIDGE_DRIVER);
	drivers.push_back(GAMEPAD_DRIVER);
	return drivers;
}

std::string MidiIO::getDriverName(int driver) {
	switch (driver) {
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
	json_object_set_new(rootJ, "driver", json_integer(driver));
	std::string deviceName = getDeviceName(device);
	if (!deviceName.empty())
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void MidiIO::fromJson(json_t *rootJ) {
	json_t *driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriver(json_integer_value(driverJ));

	json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device with equal name
		int deviceCount = getDeviceCount();
		for (int device = 0; device < deviceCount; device++) {
			if (getDeviceName(device) == deviceName) {
				setDevice(device);
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
	setDriver(-1);
}

void MidiInput::setDriver(int driver) {
	setDevice(-1);
	if (midiInputDriver) {
		midiInputDriver = NULL;
	}

	if (driver >= 0) {
		midiInputDriver = rtmidiGetInputDriver(driver);
	}
	else if (driver == BRIDGE_DRIVER) {
		// TODO
	}
	else if (driver == GAMEPAD_DRIVER) {
		midiInputDriver = gamepadGetInputDriver();
	}
	this->driver = driver;
}

int MidiInput::getDeviceCount() {
	if (midiInputDriver) {
		return midiInputDriver->getDeviceCount();
	}
	return 0;
}

std::string MidiInput::getDeviceName(int device) {
	if (midiInputDriver) {
		return midiInputDriver->getDeviceName(device);
	}
	return "";
}

void MidiInput::setDevice(int device) {
	if (midiInputDevice) {
		midiInputDevice->unsubscribe(this);
		midiInputDevice = NULL;
	}

	if (midiInputDriver && device >= 0) {
		midiInputDevice = midiInputDriver->getDevice(device);
		midiInputDevice->subscribe(this);
	}
	this->device = device;
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

void MidiOutput::setDriver(int driver) {
	// TODO
}

void MidiOutput::setDevice(int device) {
	// TODO
}


} // namespace rack
