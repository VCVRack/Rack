#pragma once

#include "bogaudio.hpp"
#include "dsp/noise.hpp"

using namespace bogaudio::dsp;

extern Model* modelSampleHold;

namespace bogaudio {

struct SampleHold : Module {
	enum ParamIds {
		TRIGGER1_PARAM,
		TRIGGER2_PARAM,
		TRACK1_PARAM,
		TRACK2_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		TRIGGER1_INPUT,
		IN1_INPUT,
		TRIGGER2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		TRACK1_LIGHT,
		TRACK2_LIGHT,
		NUM_LIGHTS
	};

	Trigger _trigger1, _trigger2;
	float _value1, _value2;
	WhiteNoiseGenerator _noise;

	SampleHold()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _value1(0.0f)
	, _value2(0.0f)
	{
		onReset();
	}

	void onReset() override;
	void step() override;
};

} // namespace bogaudio
