#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelXFade;

namespace bogaudio {

struct XFade : Module {
	enum ParamsIds {
		MIX_PARAM,
		CURVE_PARAM,
		LINEAR_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		MIX_INPUT,
		A_INPUT,
		B_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		LINEAR_LIGHT,
		NUM_LIGHTS
	};

	bool _linear = false;
	float _mix = 0.0f;
	float _curveIn = -1.0f;
	SlewLimiter _mixSL;
	CrossFader _mixer;

	XFade() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onSampleRateChange();
	}

	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
