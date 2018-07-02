#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelRM;

namespace bogaudio {

struct RM : Module {
	enum ParamsIds {
		MODULATOR_DEPTH_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		MODULATOR_INPUT,
		CARRIER_INPUT,
		MODULATOR_DEPTH_INPUT,
		MIX_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	CrossFader _mix;

	RM() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
