#pragma once

#include "bogaudio.hpp"
#include "dsp/oscillator.hpp"
#include "dsp/pitch.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelAdditator;

namespace bogaudio {

struct Additator : Module {
	enum ParamsIds {
		FREQUENCY_PARAM,
		PARTIALS_PARAM,
		WIDTH_PARAM,
		ODD_SKEW_PARAM,
		EVEN_SKEW_PARAM,
		GAIN_PARAM,
		DECAY_PARAM,
		BALANCE_PARAM,
		FILTER_PARAM,
		PHASE_PARAM,
		FINE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		PITCH_INPUT,
		SYNC_INPUT,
		PARTIALS_INPUT,
		WIDTH_INPUT,
		ODD_SKEW_INPUT,
		EVEN_SKEW_INPUT,
		GAIN_INPUT,
		DECAY_INPUT,
		BALANCE_INPUT,
		FILTER_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		SINE_LIGHT,
		COSINE_LIGHT,
		NUM_LIGHTS
	};

	enum Phase {
		PHASE_RESET,
		PHASE_SINE,
		PHASE_COSINE
	};

	const int modulationSteps = 100;
	const int maxPartials = 100;
	const float maxWidth = 2.0f;
	const float maxSkew = 0.99f;
	const float minAmplitudeNormalization = 1.0f;
	const float maxAmplitudeNormalization = 5.0f;
	const float minDecay = -1.0f;
	const float maxDecay = 3.0f;
	const float minFilter = 0.1;
	const float maxFilter = 1.9;
	const float slewLimitTime = 1.0f;

	int _steps = 0;
	int _partials = 0;
	float _width = 0.0f;
	float _oddSkew = 0.0f;
	float _evenSkew = 0.0f;
	float _amplitudeNormalization = 0.0f;
	float _decay = 0.0f;
	float _balance = 0.0f;
	float _filter = 0.0f;
	Phase _phase = PHASE_RESET;
	float _maxFrequency = 0.0f;
	int _activePartials = 1;
	SineBankOscillator _oscillator;
	PositiveZeroCrossing _syncTrigger;
	SlewLimiter _widthSL;
	SlewLimiter _oddSkewSL;
	SlewLimiter _evenSkewSL;
	SlewLimiter _amplitudeNormalizationSL;
	SlewLimiter _decaySL;
	SlewLimiter _balanceSL;
	SlewLimiter _filterSL;

	Additator()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _oscillator(1000.0f, 100.0f, maxPartials)
	{
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	float cvValue(Input& cv, bool dc = false);
	void step() override;
};

} // namespace bogaudio
