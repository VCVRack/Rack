#pragma once

#include "bogaudio.hpp"

extern Model* modelManual;

namespace bogaudio {

struct Manual : TriggerOnLoadModule {
	enum ParamsIds {
		TRIGGER_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	bool _firstStep = true;
	Trigger _trigger;
	PulseGenerator _pulse;

	Manual() : TriggerOnLoadModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		_triggerOnLoad = false;
		onReset();
	}

	void onReset() override;
	void step() override;
	bool shouldTriggerOnNextLoad() override {
		return true;
	}
};

} // namespace bogaudio
