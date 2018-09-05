#pragma once

#include "bogaudio.hpp"

extern Model* modelCmp;

namespace bogaudio {

struct Cmp : Module {
	enum ParamsIds {
		A_PARAM,
		B_PARAM,
		WINDOW_PARAM,
		LAG_PARAM,
		OUTPUT_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		A_INPUT,
		B_INPUT,
		WINDOW_INPUT,
		LAG_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		GREATER_OUTPUT,
		LESS_OUTPUT,
		EQUAL_OUTPUT,
		NOT_EQUAL_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	enum State {
		LOW,
		HIGH,
		LAG_LOW,
		LAG_HIGH
	};

	State _thresholdState;
	int _thresholdLag = 0;
	State _windowState;
	int _windowLag = 0;

	Cmp() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	void stepChannel(
		bool high,
		float highValue,
		float lowValue,
		State& state,
		int& channelLag,
		int& lag,
		Output& highOutput,
		Output& lowOutput
	);
	int lagInSamples();
};

} // namespace bogaudio
