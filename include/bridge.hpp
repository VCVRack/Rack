#pragma once
#include "bridgeprotocol.hpp"
#include "audio.hpp"


namespace rack {


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, audio::IO *audio);
void bridgeAudioUnsubscribe(int channel, audio::IO *audio);


} // namespace rack
