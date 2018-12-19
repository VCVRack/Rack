#pragma once
#include "bridgeprotocol.hpp"
#include "audio.hpp"
#include "midi.hpp"


namespace rack {


struct BridgeMidiInputDevice : midi::InputDevice {
};


struct BridgeMidiDriver : midi::Driver {
	BridgeMidiInputDevice devices[16];
	std::string getName() override {return "Bridge";}

	std::vector<int> getInputDeviceIds() override;
	std::string getInputDeviceName(int deviceId) override;
	midi::InputDevice *subscribeInputDevice(int deviceId, midi::Input *input) override;
	void unsubscribeInputDevice(int deviceId, midi::Input *input) override;
};


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, AudioIO *audio);
void bridgeAudioUnsubscribe(int channel, AudioIO *audio);


} // namespace rack
