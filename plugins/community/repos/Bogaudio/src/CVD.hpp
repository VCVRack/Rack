#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelCVD;

namespace bogaudio {

struct CVD : Module {
	enum ParamsIds {
		TIME_PARAM,
		TIME_SCALE_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		TIME_INPUT,
		MIX_INPUT,
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

	DelayLine _delay;
	CrossFader _mix;

	CVD()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _delay(1000.0f, 10000.0f)
	{
		onSampleRateChange();
	}

	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
