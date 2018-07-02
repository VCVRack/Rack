#pragma once

#include "bogaudio.hpp"
#include "shaper_core.hpp"

extern Model* modelShaperPlus;

namespace bogaudio {

struct ShaperPlus : TriggerOnLoadModule {
	enum ParamIds {
		ATTACK_PARAM,
		ON_PARAM,
		DECAY_PARAM,
		OFF_PARAM,
		ENV_PARAM,
		SIGNAL_PARAM,
		TRIGGER_PARAM,
		SPEED_PARAM,
		LOOP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT,
		TRIGGER_INPUT,
		ATTACK_INPUT,
		ON_INPUT,
		DECAY_INPUT,
		OFF_INPUT,
		ENV_INPUT,
		SIGNALCV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		ENV_OUTPUT,
		INV_OUTPUT,
		TRIGGER_OUTPUT,
		ATTACK_OUTPUT,
		ON_OUTPUT,
		DECAY_OUTPUT,
		OFF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ATTACK_LIGHT,
		ON_LIGHT,
		DECAY_LIGHT,
		OFF_LIGHT,
		NUM_LIGHTS
	};

	ShaperCore _core;

	ShaperPlus() : TriggerOnLoadModule(
		NUM_PARAMS,
		NUM_INPUTS,
		NUM_OUTPUTS,
		NUM_LIGHTS
	)
	, _core(
		params[ATTACK_PARAM],
		params[ON_PARAM],
		params[DECAY_PARAM],
		params[OFF_PARAM],
		params[ENV_PARAM],
		params[SIGNAL_PARAM],
		params[TRIGGER_PARAM],
		params[SPEED_PARAM],
		params[LOOP_PARAM],

		inputs[SIGNAL_INPUT],
		inputs[TRIGGER_INPUT],
		&inputs[ATTACK_INPUT],
		&inputs[ON_INPUT],
		&inputs[DECAY_INPUT],
		&inputs[OFF_INPUT],
		&inputs[ENV_INPUT],
		&inputs[SIGNALCV_INPUT],

		outputs[SIGNAL_OUTPUT],
		outputs[ENV_OUTPUT],
		outputs[INV_OUTPUT],
		outputs[TRIGGER_OUTPUT],
		&outputs[ATTACK_OUTPUT],
		&outputs[ON_OUTPUT],
		&outputs[DECAY_OUTPUT],
		&outputs[OFF_OUTPUT],

		lights[ATTACK_LIGHT],
		lights[ON_LIGHT],
		lights[DECAY_LIGHT],
		lights[OFF_LIGHT],

		_triggerOnLoad,
		_shouldTriggerOnLoad
	)
	{
		onReset();
	}

	void onReset() override {
		_core.reset();
	}

	void step() override {
		_core.step();
	}

	bool shouldTriggerOnNextLoad() override {
		return _core._stage != _core.STOPPED_STAGE;
	}
};

} // namespace bogaudio
