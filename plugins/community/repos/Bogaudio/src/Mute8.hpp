#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelMute8;

namespace bogaudio {

struct Mute8 : Module {
	enum ParamsIds {
		MUTE1_PARAM,
		MUTE2_PARAM,
		MUTE3_PARAM,
		MUTE4_PARAM,
		MUTE5_PARAM,
		MUTE6_PARAM,
		MUTE7_PARAM,
		MUTE8_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		INPUT1_INPUT,
		INPUT2_INPUT,
		INPUT3_INPUT,
		INPUT4_INPUT,
		INPUT5_INPUT,
		INPUT6_INPUT,
		INPUT7_INPUT,
		INPUT8_INPUT,
		MUTE1_INPUT,
		MUTE2_INPUT,
		MUTE3_INPUT,
		MUTE4_INPUT,
		MUTE5_INPUT,
		MUTE6_INPUT,
		MUTE7_INPUT,
		MUTE8_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUTPUT1_OUTPUT,
		OUTPUT2_OUTPUT,
		OUTPUT3_OUTPUT,
		OUTPUT4_OUTPUT,
		OUTPUT5_OUTPUT,
		OUTPUT6_OUTPUT,
		OUTPUT7_OUTPUT,
		OUTPUT8_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		MUTE1_LIGHT,
		MUTE2_LIGHT,
		MUTE3_LIGHT,
		MUTE4_LIGHT,
		MUTE5_LIGHT,
		MUTE6_LIGHT,
		MUTE7_LIGHT,
		MUTE8_LIGHT,
		NUM_LIGHTS
	};

	static const float maxDecibels;
	static const float minDecibels;
	static const float slewTimeMS;

	Amplifier _amplifiers[8];
	SlewLimiter _slewLimiters[8];
	Trigger _triggers[8];

	Mute8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
	void stepChannel(int i, bool solo);
};

} // namespace bogaudio
