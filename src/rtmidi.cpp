#include "rtmidi.hpp"
#include <map>


namespace rack {


RtMidiInputDriver::RtMidiInputDriver(int driverId) {
	rtMidiIn = new RtMidiIn((RtMidi::Api) driverId);
	assert(rtMidiIn);
}

RtMidiInputDriver::~RtMidiInputDriver() {
	delete rtMidiIn;
}

std::vector<int> RtMidiInputDriver::getDeviceIds() {
	int count = rtMidiIn->getPortCount();;
	std::vector<int> deviceIds;
	for (int i = 0; i < count; i++)
		deviceIds.push_back(i);
	return deviceIds;
}

std::string RtMidiInputDriver::getDeviceName(int deviceId) {
	if (deviceId >= 0) {
		return rtMidiIn->getPortName(deviceId);
	}
	return "";
}

MidiInputDevice *RtMidiInputDriver::getDevice(int deviceId) {
	// TODO Get from cache
	return new RtMidiInputDevice(rtMidiIn->getCurrentApi(), deviceId);
}


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

RtMidiInputDevice::RtMidiInputDevice(int driverId, int deviceId) {
	rtMidiIn = new RtMidiIn((RtMidi::Api) driverId, "VCV Rack");
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

static std::map<int, RtMidiInputDriver*> rtmidiInputDrivers;

MidiInputDriver *rtmidiGetInputDriver(int driverId) {
	// Lazily create RtMidiInputDriver
	RtMidiInputDriver *d = rtmidiInputDrivers[driverId];
	if (!d) {
		rtmidiInputDrivers[driverId] = d = new RtMidiInputDriver(driverId);
	}
	return d;
}


} // namespace rack
