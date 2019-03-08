#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelLmtr;

namespace bogaudio {

struct Lmtr : Module {
	enum ParamsIds {
		THRESHOLD_PARAM,
		OUTPUT_GAIN_PARAM,
		KNEE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		LEFT_INPUT,
		RIGHT_INPUT,
		THRESHOLD_INPUT,
		OUTPUT_GAIN_INPUT,
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
	float _outGain = -1.0f;
	float _outLevel = 0.0f;
	bool _softKnee = true;
	float _lastEnv = 0.0f;

	SlewLimiter _attackSL;
	SlewLimiter _releaseSL;
	RootMeanSquare _detector;
	Compressor _compressor;
	Amplifier _amplifier;
	Saturator _saturator;

	Lmtr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
