#pragma once

#include "bogaudio.hpp"
#include "dsp/envelope.hpp"

extern Model* modelADSR;

namespace bogaudio {

struct ADSR : Module {
	enum ParamsIds {
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		LINEAR_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		GATE_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		ATTACK_LIGHT,
		DECAY_LIGHT,
		SUSTAIN_LIGHT,
		RELEASE_LIGHT,
		LINEAR_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	bool _linearMode = false;
	Trigger _gateTrigger;
	bogaudio::dsp::ADSR _envelope;

	ADSR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void step() override;
};

} // namespace bogaudio
