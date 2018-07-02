#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelPan;

namespace bogaudio {

struct Pan : Module {
	enum ParamsIds {
		PAN1_PARAM,
		PAN2_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		L_OUTPUT,
		R_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	Panner _panner1;
	Panner _panner2;
	SlewLimiter _slew1;
	SlewLimiter _slew2;
	Saturator _saturatorLeft;
	Saturator _saturatorRight;

	Pan() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onSampleRateChange();
	}

	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
