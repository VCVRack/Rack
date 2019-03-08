#pragma once

#include "bogaudio.hpp"
#include "dsp/oscillator.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelLLFO;

namespace bogaudio {

struct LLFO : Module {
	enum ParamsIds {
		FREQUENCY_PARAM,
		SLOW_PARAM,
		WAVE_PARAM,
		OFFSET_PARAM,
		SCALE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		PITCH_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		SLOW_LIGHT,
		SINE_LIGHT,
		RAMP_UP_LIGHT,
		SQUARE_LIGHT,
		TRIANGLE_LIGHT,
		RAMP_DOWN_LIGHT,
		PULSE_LIGHT,
		NUM_LIGHTS
	};

	enum Wave {
		SINE_WAVE,
		TRIANGLE_WAVE,
		RAMP_UP_WAVE,
		RAMP_DOWN_WAVE,
		SQUARE_WAVE,
		PULSE_WAVE
	};

	const int modulationSteps = 100;
	const float amplitude = 5.0f;

	int _modulationStep = 0;
	bool _slowMode = false;
	float _offset = 0.0f;
	float _scale = 0.0f;
	PositiveZeroCrossing _resetTrigger;

	Phasor _phasor;
	SineTableOscillator _sine;
	TriangleOscillator _triangle;
	SawOscillator _ramp;
	SquareOscillator _square;

	Wave _wave;
	bool _invert;
	Phasor* _oscillator;

	LLFO()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _wave(SINE_WAVE)
	, _invert(false)
	, _oscillator(&_sine)
	{
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
