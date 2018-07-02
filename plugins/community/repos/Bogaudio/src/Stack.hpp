#pragma once

#include "bogaudio.hpp"
#include "dsp/pitch.hpp"

using namespace bogaudio::dsp;

extern Model* modelStack;

namespace bogaudio {

struct Stack : Module {
	enum ParamsIds {
		SEMIS_PARAM,
		OCTAVE_PARAM,
		FINE_PARAM,
		QUANTIZE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		THRU_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		QUANTIZE_LIGHT,
		NUM_LIGHTS
	};

	const float _minCVOut = semitoneToCV(24.0); // C1
	const float _maxCVOut = semitoneToCV(120.0); // C9

	float _semitones = -1000.0f;
	float _inCV = -1000.0f;
	float _fine = -1000.0f;
	float _outCV;

	Stack() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

} // namespace bogaudio
