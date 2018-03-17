#pragma once
#include "bridgeprotocol.hpp"
#include "audio.hpp"


namespace rack {


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, AudioIO *audio);
void bridgeAudioUnsubscribe(int channel, AudioIO *audio);


} // namespace rack
