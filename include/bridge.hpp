#pragma once
#include "audio.hpp"


namespace rack {


static const int BRIDGE_CHANNELS = 16;


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, AudioIO *audio);
void bridgeAudioUnsubscribe(int channel, AudioIO *audio);
bool bridgeAudioIsActive(int channel, AudioIO *audio);


} // namespace rack
