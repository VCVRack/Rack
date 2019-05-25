#pragma once
#include <bridgeprotocol.hpp>
#include <audio.hpp>


namespace rack {


void bridgeInit();
void bridgeDestroy();
void bridgeAudioSubscribe(int channel, audio::Port *port);
void bridgeAudioUnsubscribe(int channel, audio::Port *port);


} // namespace rack
