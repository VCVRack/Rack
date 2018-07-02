#include "global_pre.hpp"
#include "vstmidi.hpp"
#include <map>
#include "global.hpp"


extern void vst2_lock_midi_device(void);
extern void vst2_unlock_midi_device(void);


namespace rack {


// static void midiInputCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
// 	if (!message) return;
// 	if (!userData) return;

// 	RtMidiInputDevice *midiInputDevice = (RtMidiInputDevice*) userData;
// 	if (!midiInputDevice) return;
// 	MidiMessage msg;
// 	if (message->size() >= 1)
// 		msg.cmd = (*message)[0];
// 	if (message->size() >= 2)
// 		msg.data1 = (*message)[1];
// 	if (message->size() >= 3)
// 		msg.data2 = (*message)[2];

// 	midiInputDevice->onMessage(msg);
// }

VSTMidiInputDevice::VSTMidiInputDevice(int driverId, int deviceId) {
}

VSTMidiInputDevice::~VSTMidiInputDevice() {
}


VSTMidiDriver::VSTMidiDriver(int driverId) {
   device = NULL;
}

VSTMidiDriver::~VSTMidiDriver() {
   if(NULL != device)
      delete device;
}

std::string VSTMidiDriver::getName() {
   return "VST";
}

std::vector<int> VSTMidiDriver::getInputDeviceIds() {
	std::vector<int> deviceIds;
   deviceIds.push_back(0);
	return deviceIds;
}

std::string VSTMidiDriver::getInputDeviceName(int deviceId) {
	if(0 == deviceId)
		return std::string("VST MIDI Input Device");
	return "";
}

MidiInputDevice *VSTMidiDriver::subscribeInputDevice(int deviceId, MidiInput *midiInput) {
	if(0 != deviceId)
		return NULL;

	if (!device) {
		device = new VSTMidiInputDevice(VST_DRIVER, deviceId);
      vst2_lock_midi_device();
      global->vst2.midi_device = device;
      vst2_unlock_midi_device();
	}

	device->subscribe(midiInput);
	return device;
}

void VSTMidiDriver::unsubscribeInputDevice(int deviceId, MidiInput *midiInput) {
   if(0 != deviceId)
      return;
	device->unsubscribe(midiInput);

	// Destroy device if nothing is subscribed anymore
	if (device->subscribed.empty()) {
      vst2_lock_midi_device();
		delete device;
      device = NULL;
      global->vst2.midi_device = NULL;
      vst2_unlock_midi_device();
	}
}

void vstmidiInit() {
   MidiDriver *driver = new VSTMidiDriver(VST_DRIVER);
   midiDriverAdd(VST_DRIVER, driver);
}

} // namespace rack

void vst2_process_midi_input_event(sU8 _a, sU8 _b, sU8 _c) {
   // (note) vst midi device mutex is locked by caller
   if(NULL != rack::global->vst2.midi_device)
   {
      rack::MidiMessage msg;
      msg.cmd   = _a;
      msg.data1 = _b;
      msg.data2 = _c;

      rack::global->vst2.midi_device->onMessage(msg);
   }
}
