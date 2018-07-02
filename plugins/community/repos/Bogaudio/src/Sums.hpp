#pragma once

#include "bogaudio.hpp"
#include "disable_output_limit.hpp"

extern Model* modelSums;

namespace bogaudio {

struct Sums : DisableOutputLimitModule {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		A_INPUT,
		B_INPUT,
		NEGATE_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		SUM_OUTPUT,
		DIFFERENCE_OUTPUT,
		MAX_OUTPUT,
		MIN_OUTPUT,
		NEGATE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	Sums() : DisableOutputLimitModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
