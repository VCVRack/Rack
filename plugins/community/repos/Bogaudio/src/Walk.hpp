#pragma once

#include "bogaudio.hpp"
#include "dsp/noise.hpp"

using namespace bogaudio::dsp;

extern Model* modelWalk;

namespace bogaudio {

struct Walk : Module {
	enum ParamsIds {
		RATE_PARAM,
		OFFSET_PARAM,
		SCALE_PARAM,
		TRACK_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		RATE_INPUT,
		HOLD_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		HOLD_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		TRACK_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	Trigger _trigger;
	RandomWalk _walk;
	float _hold = 0.0f;

	Walk() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
