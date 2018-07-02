#pragma once

#include "bogaudio.hpp"
#include "dsp/filter.hpp"
#include "dsp/oscillator.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelVCO;

namespace bogaudio {

struct VCO : Module {
	enum ParamsIds {
		FREQUENCY_PARAM,
		FINE_PARAM,
		SLOW_PARAM,
		PW_PARAM,
		FM_PARAM,
		FM_TYPE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		PITCH_INPUT,
		SYNC_INPUT,
		PW_INPUT,
		FM_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		SQUARE_OUTPUT,
		SAW_OUTPUT,
		TRIANGLE_OUTPUT,
		SINE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		SLOW_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	const float amplitude = 5.0f;
	static constexpr int oversample = 8;
	int _modulationStep = 0;
	float _oversampleThreshold = 0.0f;
	float _frequency = 0.0f;
	float _baseVOct = 0.0f;
	float _baseHz = 0.0f;
	bool _slowMode = false;
	float _fmDepth = 0.0f;
	bool _fmLinearMode = false;

	Phasor _phasor;
	BandLimitedSquareOscillator _square;
	BandLimitedSawOscillator _saw;
	TriangleOscillator _triangle;
	SineTableOscillator _sine;
	CICDecimator _squareDecimator;
	CICDecimator _sawDecimator;
	CICDecimator _triangleDecimator;
	float _squareBuffer[oversample];
	float _sawBuffer[oversample];
	float _triangleBuffer[oversample];
	PositiveZeroCrossing _syncTrigger;
	SlewLimiter _squarePulseWidthSL;

	VCO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		setSampleRate(engineGetSampleRate());
		_saw.setQuality(12);
		_square.setQuality(12);
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
	void setSampleRate(float sampleRate);
	void setFrequency(float frequency);
};

} // namespace bogaudio
