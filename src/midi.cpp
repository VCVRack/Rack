#include "midi.hpp"


namespace rack {


int MidiIO::getPortCount() {
	return midi->getPortCount();
}

std::string MidiIO::getPortName(int port) {
	return midi->getPortName(port);
}

void MidiIO::openPort(int port) {
	midi->closePort();

	if (port >= 0) {
		midi->openPort(port);
	}
	this->port = port;
}

MidiInput::MidiInput() {
	midi = new RtMidiIn();
}

MidiInput::~MidiInput() {
	delete dynamic_cast<RtMidiIn*>(midi);
}


MidiOutput::MidiOutput() {
	midi = new RtMidiOut();
}

MidiOutput::~MidiOutput() {
	delete dynamic_cast<RtMidiOut*>(midi);
}


} // namespace rack
