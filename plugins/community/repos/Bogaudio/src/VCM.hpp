#pragma once

#include "bogaudio.hpp"
#include "disable_output_limit.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelVCM;

namespace bogaudio {

struct VCM : DisableOutputLimitModule {
	enum ParamsIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
		LEVEL3_PARAM,
		LEVEL4_PARAM,
		MIX_PARAM,
		LINEAR_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		IN1_INPUT,
		CV1_INPUT,
		IN2_INPUT,
		CV2_INPUT,
		IN3_INPUT,
		CV3_INPUT,
		IN4_INPUT,
		CV4_INPUT,
		MIX_CV_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		MIX_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		LINEAR_LIGHT,
		NUM_LIGHTS
	};

	Amplifier _amplifier1;
	Amplifier _amplifier2;
	Amplifier _amplifier3;
	Amplifier _amplifier4;

	VCM() : DisableOutputLimitModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void step() override;
	float channelStep(Input& input, Param& knob, Input& cv, Amplifier& amplifier, bool linear);
};

} // namespace bogaudio
