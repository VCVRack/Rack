#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelVCA;

namespace bogaudio {

struct VCA : Module {
	enum ParamsIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
		LINEAR_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		LINEAR_LIGHT,
		NUM_LIGHTS
	};

	Amplifier _amplifier1;
	SlewLimiter _levelSL1;
	Amplifier _amplifier2;
	SlewLimiter _levelSL2;

	VCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onSampleRateChange();
	}

	void onSampleRateChange() override;
	void step() override;
	void channelStep(Input& input, Output& output, Param& knob, Input& cv, Amplifier& amplifier, SlewLimiter& levelSL, bool linear);
};

} // namespace bogaudio
