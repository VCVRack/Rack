#pragma once

#include "bogaudio.hpp"
#include "dsp/oscillator.hpp"
#include "dsp/pitch.hpp"

using namespace bogaudio::dsp;

extern Model* modelReftone;

namespace bogaudio {

struct Reftone : Module {
	enum ParamsIds {
		PITCH_PARAM,
		OCTAVE_PARAM,
		FINE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		NUM_INPUTS
	};

	enum OutputsIds {
		CV_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	int _pitch = 9;
	int _octave = 4;
	float _fine = 0.0f;
	float _frequency = 440.0f;
	float _cv = frequencyToCV(_frequency);
	SineOscillator _sine;

	Reftone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		_sine.setSampleRate(engineGetSampleRate());
	}

	void step() override;
};

} // namespace bogaudio
