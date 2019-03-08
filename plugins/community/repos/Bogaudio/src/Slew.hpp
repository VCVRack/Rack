#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelSlew;

namespace bogaudio {

struct Slew : Module {
	enum ParamsIds {
		RISE_PARAM,
		RISE_SHAPE_PARAM,
		FALL_PARAM,
		FALL_SHAPE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		RISE_INPUT,
		FALL_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	bool _rising = true;
	float _last = 0.0f;
	ShapedSlewLimiter _rise;
	ShapedSlewLimiter _fall;

	Slew() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	float time(Param& param, Input& input);
	float shape(Param& param);
};

} // namespace bogaudio
