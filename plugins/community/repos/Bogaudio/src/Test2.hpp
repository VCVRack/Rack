#pragma once

#include "bogaudio.hpp"

extern Model* modelTest2;

// #define COMPLEX_BIQUAD 1
// #define MULTIPOLE 1
// #define ADSR_ENVELOPE 1
#define LIMITER 1

#ifdef COMPLEX_BIQUAD
#include "dsp/filter.hpp"
#elif MULTIPOLE
#include "dsp/filter.hpp"
#elif ADSR_ENVELOPE
#include "dsp/envelope.hpp"
#elif LIMITER
#include "dsp/signal.hpp"
#elif
#error what
#endif

using namespace bogaudio::dsp;

namespace bogaudio {

struct Test2 : Module {
	enum ParamsIds {
		PARAM1A_PARAM,
		PARAM2A_PARAM,
		PARAM3A_PARAM,
		PARAM1B_PARAM,
		PARAM2B_PARAM,
		PARAM3B_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1A_INPUT,
		CV2A_INPUT,
		CV3A_INPUT,
		CV1B_INPUT,
		CV2B_INPUT,
		CV3B_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

#ifdef COMPLEX_BIQUAD
	ComplexBiquadFilter _complexBiquad;
#elif MULTIPOLE
	MultipoleFilter _filter;
	const int maxSteps = 10;
	int _steps = maxSteps;
#elif ADSR_ENVELOPE
	ADSR _adsr;
	Trigger _trigger;
#elif LIMITER
	Limiter _limiter;
#endif

	Test2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	// float oscillatorPitch1A();
};

} // namespace bogaudio
