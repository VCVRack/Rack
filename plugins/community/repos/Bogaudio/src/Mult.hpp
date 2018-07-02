#pragma once

#include "bogaudio.hpp"

extern Model* modelMult;

namespace bogaudio {

struct Mult : Module {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		INA_INPUT,
		INB_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUTA1_OUTPUT,
		OUTA2_OUTPUT,
		OUTA3_OUTPUT,
		OUTB1_OUTPUT,
		OUTB2_OUTPUT,
		OUTB3_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	Mult() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
