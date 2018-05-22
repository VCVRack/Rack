#pragma once
#include "bridgeprotocol.hpp"
#include "audio.hpp"
#include "midi.hpp"


namespace rack {


struct BridgeMidiInputDevice : MidiInputDevice {
};


struct BridgeMidiDriver : MidiDriver {
	BridgeMidiInputDevice devices[16];
	std::string getName() override {return "Bridge";}

	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	MidiInputDevice *subscribeInputDevice(int deviceId, MidiInput *midiInput) override;
	void unsubscribeInputDevice(int deviceId, MidiInput *midiInput) override;
};


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, AudioIO *audio);
void bridgeAudioUnsubscribe(int channel, AudioIO *audio);


} // namespace rack
