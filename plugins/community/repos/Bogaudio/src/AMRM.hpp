#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelAMRM;

namespace bogaudio {

struct AMRM : Module {
	enum ParamsIds {
		RECTIFY_PARAM,
		DRYWET_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		MODULATOR_INPUT,
		CARRIER_INPUT,
		RECTIFY_INPUT,
		DRYWET_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		RECTIFY_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	Saturator _saturator;

	AMRM() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
