#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelNsgt;

namespace bogaudio {

struct Nsgt : Module {
	enum ParamsIds {
		THRESHOLD_PARAM,
		RATIO_PARAM,
		KNEE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		LEFT_INPUT,
		RIGHT_INPUT,
		THRESHOLD_INPUT,
		RATIO_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	float _thresholdDb = 0.0f;
	float _ratio = 0.0f;
	float _ratioKnob = -1.0f;
	bool _softKnee = true;
	float _lastEnv = 0.0f;

	SlewLimiter _attackSL;
	SlewLimiter _releaseSL;
	RootMeanSquare _detector;
	NoiseGate _noiseGate;
	Amplifier _amplifier;
	Saturator _saturator;

	Nsgt() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
