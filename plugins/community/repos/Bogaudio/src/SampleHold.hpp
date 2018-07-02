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
		NUM_PARAMS
	};

	enum InputIds {
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

	SchmittTrigger _trigger1, _trigger2;
	float _value1, _value2;
	WhiteNoiseGenerator _noise;

	SampleHold() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	void step(
		Param& triggerParam,
		Input& triggerInput,
		Input& in,
		Output& out,
		SchmittTrigger& trigger,
		float& value
	);
};

} // namespace bogaudio
