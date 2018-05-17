#include "rtmidi.hpp"


namespace rack {


static void midiInputCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
	if (!message) return;
	if (!userData) return;

	RtMidiInputDevice *midiInputDevice = (RtMidiInputDevice*) userData;
	if (!midiInputDevice) return;
	MidiMessage msg;
	if (message->size() >= 1)
		msg.cmd = (*message)[0];
	if (message->size() >= 2)
		msg.data1 = (*message)[1];
	if (message->size() >= 3)
		msg.data2 = (*message)[2];

	midiInputDevice->onMessage(msg);
}


RtMidiInputDevice::RtMidiInputDevice(int driver, int device) {
	rtMidiIn = new RtMidiIn((RtMidi::Api) driver, "VCV Rack");
	rtMidiIn->ignoreTypes(false, false, false);
	rtMidiIn->setCallback(midiInputCallback, this);
}


RtMidiInputDevice::~RtMidiInputDevice() {
	rtMidiIn->closePort();
	delete rtMidiIn;
}


std::vector<int> rtmidiGetDrivers() {
	std::vector<RtMidi::Api> rtApis;
	RtMidi::getCompiledApi(rtApis);

	std::vector<int> drivers;
	for (RtMidi::Api api : rtApis) {
		drivers.push_back((int) api);
	}
	return drivers;
}

int rtmidiGetDeviceCount(int driver) {
	RtMidiIn *rtMidiIn = new RtMidiIn((RtMidi::Api) driver);
	int count = rtMidiIn->getPortCount();
	delete rtMidiIn;
	return count;
}

std::string rtmidiGetDeviceName(int driver, int device) {
	RtMidiIn *rtMidiIn = new RtMidiIn((RtMidi::Api) driver);
	std::string name = rtMidiIn->getPortName(device);
	delete rtMidiIn;
	return name;
}

RtMidiInputDevice *rtmidiGetDevice(int driver, int device) {
	// TODO Pull from cache
	return new RtMidiInputDevice(driver, device);
}


} // namespace rack
