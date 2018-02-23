#include "midi.hpp"


namespace rack {


////////////////////
// MidiIO
////////////////////

MidiIO::~MidiIO() {
	setDriver(-1);
}

std::vector<int> MidiIO::getDrivers() {
	std::vector<RtMidi::Api> rtApis;
	RtMidi::getCompiledApi(rtApis);

	std::vector<int> drivers;
	for (RtMidi::Api api : rtApis) {
		drivers.push_back((int) api);
	}
	// drivers.push_back(BRIDGE_DRIVER)
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
		// case BRIDGE_DRIVER: return "Bridge";
		default: return "Unknown";
	}
}

int MidiIO::getDeviceCount() {
	if (rtMidi) {
		return rtMidi->getPortCount();
	}
	return 0;
}

std::string MidiIO::getDeviceName(int device) {
	if (rtMidi) {
		if (device < 0)
			return "";
		if (device == this->device)
			return deviceName;
		else
			return rtMidi->getPortName(device);
	}
	return "";
}

void MidiIO::setDevice(int device) {
	if (rtMidi) {
		rtMidi->closePort();

		if (device >= 0) {
			rtMidi->openPort(device);
			deviceName = rtMidi->getPortName(device);
		}
		this->device = device;
	}
}

std::string MidiIO::getChannelName(int channel) {
	if (channel == -1)
		return "All channels";
	else
		return stringf("Channel %d", channel + 1);
}

bool MidiIO::isActive() {
	if (rtMidi)
		return rtMidi->isPortOpen();
	return false;
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

static void midiInputCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
	if (!message) return;
	if (!userData) return;

	MidiInput *midiInput = (MidiInput*) userData;
	if (!midiInput) return;
	MidiMessage midiMessage;
	if (message->size() >= 1)
		midiMessage.cmd = (*message)[0];
	if (message->size() >= 2)
		midiMessage.data1 = (*message)[1];
	if (message->size() >= 3)
		midiMessage.data2 = (*message)[2];
	midiInput->onMessage(midiMessage);
}

MidiInput::MidiInput() {
	setDriver(RtMidi::UNSPECIFIED);
}

void MidiInput::setDriver(int driver) {
	setDevice(-1);
	if (rtMidiIn) {
		delete rtMidiIn;
		rtMidi = rtMidiIn = NULL;
	}

	if (driver >= 0) {
		rtMidiIn = new RtMidiIn((RtMidi::Api) driver);
		rtMidiIn->setCallback(midiInputCallback, this);
		rtMidi = rtMidiIn;
		this->driver = rtMidiIn->getCurrentApi();
	}
}

void MidiInputQueue::onMessage(const MidiMessage &message) {
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
	setDriver(RtMidi::UNSPECIFIED);
}

void MidiOutput::setDriver(int driver) {
	setDevice(-1);
	if (rtMidiOut) {
		delete rtMidiOut;
		rtMidi = rtMidiOut = NULL;
	}

	if (driver >= 0) {
		rtMidiOut = new RtMidiOut((RtMidi::Api) driver);
		rtMidi = rtMidiOut;
		this->driver = rtMidiOut->getCurrentApi();
	}
}


} // namespace rack
