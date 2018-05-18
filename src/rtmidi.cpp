#include "rtmidi.hpp"
#include <map>


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

RtMidiInputDevice::RtMidiInputDevice(int driverId, int deviceId) {
	rtMidiIn = new RtMidiIn((RtMidi::Api) driverId, "VCV Rack");
	assert(rtMidiIn);
	rtMidiIn->ignoreTypes(false, false, false);
	rtMidiIn->setCallback(midiInputCallback, this);
}

RtMidiInputDevice::~RtMidiInputDevice() {
	rtMidiIn->closePort();
	delete rtMidiIn;
}


RtMidiDriver::RtMidiDriver(int driverId) {
	this->driverId = driverId;
	rtMidiIn = new RtMidiIn((RtMidi::Api) driverId);
	assert(rtMidiIn);
	rtMidiOut = new RtMidiOut((RtMidi::Api) driverId);
	assert(rtMidiOut);
}

RtMidiDriver::~RtMidiDriver() {
	delete rtMidiIn;
	delete rtMidiOut;
}

std::string RtMidiDriver::getName() {
	switch (driverId) {
		case RtMidi::UNSPECIFIED: return "Unspecified";
		case RtMidi::MACOSX_CORE: return "Core MIDI";
		case RtMidi::LINUX_ALSA: return "ALSA";
		case RtMidi::UNIX_JACK: return "JACK";
		case RtMidi::WINDOWS_MM: return "Windows MIDI";
		case RtMidi::RTMIDI_DUMMY: return "Dummy MIDI";
		default: return "";
	}
}

std::vector<int> RtMidiDriver::getInputDeviceIds() {
	// TODO The IDs unfortunately jump around in RtMidi. Is there a way to keep them constant when a MIDI device is added/removed?
	int count = rtMidiIn->getPortCount();
	std::vector<int> deviceIds;
	for (int i = 0; i < count; i++)
		deviceIds.push_back(i);
	return deviceIds;
}

std::string RtMidiDriver::getInputDeviceName(int deviceId) {
	if (deviceId >= 0) {
		return rtMidiIn->getPortName(deviceId);
	}
	return "";
}

MidiInputDevice *RtMidiDriver::subscribeInputDevice(int deviceId, MidiInput *midiInput) {
	RtMidiInputDevice *device = devices[deviceId];
	if (!device) {
		devices[deviceId] = device = new RtMidiInputDevice(driverId, deviceId);
	}

	device->subscribe(midiInput);
	return device;
}

void RtMidiDriver::unsubscribeInputDevice(int deviceId, MidiInput *midiInput) {
	RtMidiInputDevice *device = devices[deviceId];
	assert(device);
	device->unsubscribe(midiInput);

	// Destroy device if nothing else is subscribed
	if (device->subscribed.empty()) {
		auto it = devices.find(deviceId);
		assert(it != devices.end());
		devices.erase(it);
		delete device;
	}
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

RtMidiDriver *rtmidiCreateDriver(int driverId) {
	return new RtMidiDriver(driverId);
}


} // namespace rack
