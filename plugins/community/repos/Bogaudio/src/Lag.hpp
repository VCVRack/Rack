#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelLag;

namespace bogaudio {

struct Lag : Module {
	enum ParamsIds {
		TIME_PARAM,
		TIME_SCALE_PARAM,
		SHAPE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		TIME_INPUT,
		SHAPE_INPUT,
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
	ShapedSlewLimiter _slew;

	Lag() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
};

} // namespace bogaudio
