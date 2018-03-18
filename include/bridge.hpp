#pragma once
#include "bridgeprotocol.hpp"
#include "audio.hpp"
#include "midi.hpp"


namespace rack {


void bridgeInit();
void bridgeDestroy();
void bridgeMidiSubscribe(int channel, MidiIO *midi);
void bridgeMidiUnsubscribe(int channel, MidiIO *midi);
void bridgeAudioSubscribe(int channel, AudioIO *audio);
void bridgeAudioUnsubscribe(int channel, AudioIO *audio);


} // namespace rack
