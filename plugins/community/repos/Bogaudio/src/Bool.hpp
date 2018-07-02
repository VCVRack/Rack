#pragma once

#include "bogaudio.hpp"

extern Model* modelBool;

namespace bogaudio {

struct Bool : Module {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		A_INPUT,
		B_INPUT,
		NOT_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		AND_OUTPUT,
		OR_OUTPUT,
		XOR_OUTPUT,
		NOT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	Bool() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
