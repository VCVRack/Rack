#include "midi.hpp"


namespace rack {


////////////////////
// MidiIO
////////////////////

int MidiIO::getPortCount() {
	return rtMidi->getPortCount();
}

std::string MidiIO::getPortName(int port) {
	if (port < 0)
		return "";
	return rtMidi->getPortName(port);
}

void MidiIO::openPort(int port) {
	rtMidi->closePort();

	if (port >= 0) {
		rtMidi->openPort(port);
	}
	this->port = port;
}

json_t *MidiIO::toJson() {
	json_t *rootJ = json_object();
	std::string portName = getPortName(port);
	json_object_set_new(rootJ, "port", json_string(portName.c_str()));
	json_object_set_new(rootJ, "channel", json_integer(channel));
	return rootJ;
}

void MidiIO::fromJson(json_t *rootJ) {
	json_t *portNameJ = json_object_get(rootJ, "port");
	if (portNameJ) {
		std::string portName = json_string_value(portNameJ);
		// Search for port with equal name
		for (int port = 0; port < getPortCount(); port++) {
			if (getPortName(port) == portName) {
				openPort(port);
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
	RtMidiIn *rtMidiIn = new RtMidiIn();
	rtMidi = rtMidiIn;
	rtMidiIn->setCallback(midiInputCallback, this);
}

MidiInput::~MidiInput() {
	delete dynamic_cast<RtMidiIn*>(rtMidi);
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
	rtMidi = new RtMidiOut();
}

MidiOutput::~MidiOutput() {
	delete dynamic_cast<RtMidiOut*>(rtMidi);
}


} // namespace rack
