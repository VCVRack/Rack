#include "midi.hpp"


namespace rack {


////////////////////
// MidiIO
////////////////////

int MidiIO::getDeviceCount() {
	if (rtMidi)
		return rtMidi->getPortCount();
	return 0;
}

std::string MidiIO::getDeviceName(int device) {
	if (rtMidi) {
		if (device < 0)
			return "";
		return rtMidi->getPortName(device);
	}
	return "";
}

void MidiIO::openDevice(int device) {
	if (rtMidi) {
		rtMidi->closePort();

		if (device >= 0) {
			rtMidi->openPort(device);
		}
		this->device = device;
	}
}

bool MidiIO::isActive() {
	if (rtMidi)
		return rtMidi->isPortOpen();
	return false;
}

json_t *MidiIO::toJson() {
	json_t *rootJ = json_object();
	std::string deviceName = getDeviceName(device);
	json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void MidiIO::fromJson(json_t *rootJ) {
	json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device with equal name
		for (int device = 0; device < getDeviceCount(); device++) {
			if (getDeviceName(device) == deviceName) {
				openDevice(device);
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
	midiMessage.time = timeStamp;
	midiMessage.data = *message;
	midiInput->onMessage(midiMessage);
}

MidiInput::MidiInput() {
	rtMidiIn = new RtMidiIn();
	rtMidi = rtMidiIn;
	rtMidiIn->setCallback(midiInputCallback, this);
}

MidiInput::~MidiInput() {
	delete rtMidiIn;
}

void MidiInputQueue::onMessage(const MidiMessage &message) {
	for (uint8_t d : message.data) {
		debug("MIDI message: %02x", d);
	}

	const int messageQueueSize = 8192;
	if (messageQueue.size() < messageQueueSize)
		messageQueue.push(message);
}

////////////////////
// MidiOutput
////////////////////

MidiOutput::MidiOutput() {
	rtMidiOut = new RtMidiOut();
	rtMidi = rtMidiOut;
}

MidiOutput::~MidiOutput() {
	delete rtMidiOut;
}


} // namespace rack
