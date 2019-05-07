#pragma once

#include "bogaudio.hpp"

extern Model* modelEightOne;

namespace bogaudio {

struct EightOne : Module {
	enum ParamsIds {
		STEPS_PARAM,
		DIRECTION_PARAM,
		SELECT_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		IN5_INPUT,
		IN6_INPUT,
		IN7_INPUT,
		IN8_INPUT,
		CLOCK_INPUT,
		RESET_INPUT,
		SELECT_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		IN1_LIGHT,
		IN2_LIGHT,
		IN3_LIGHT,
		IN4_LIGHT,
		IN5_LIGHT,
		IN6_LIGHT,
		IN7_LIGHT,
		IN8_LIGHT,
		NUM_LIGHTS
	};

	Trigger _clock;
	Trigger _reset;
	int _step;

	EightOne() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
};

} // namespace bogaudio
