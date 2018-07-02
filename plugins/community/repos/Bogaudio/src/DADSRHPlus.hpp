#pragma once

#include "bogaudio.hpp"
#include "dadsrh_core.hpp"

extern Model* modelDADSRHPlus;

namespace bogaudio {

struct DADSRHPlus : TriggerOnLoadModule {
	enum ParamsIds {
		DELAY_PARAM,
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		HOLD_PARAM,
		ATTACK_SHAPE_PARAM,
		DECAY_SHAPE_PARAM,
		RELEASE_SHAPE_PARAM,
		TRIGGER_PARAM,
		MODE_PARAM,
		LOOP_PARAM,
		SPEED_PARAM,
		RETRIGGER_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		DELAY_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		HOLD_INPUT,
		TRIGGER_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		DELAY_OUTPUT,
		ATTACK_OUTPUT,
		DECAY_OUTPUT,
		SUSTAIN_OUTPUT,
		RELEASE_OUTPUT,
		ENV_OUTPUT,
		INV_OUTPUT,
		TRIGGER_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		DELAY_LIGHT,
		ATTACK_LIGHT,
		DECAY_LIGHT,
		SUSTAIN_LIGHT,
		RELEASE_LIGHT,
		ATTACK_SHAPE1_LIGHT,
		ATTACK_SHAPE2_LIGHT,
		ATTACK_SHAPE3_LIGHT,
		DECAY_SHAPE1_LIGHT,
		DECAY_SHAPE2_LIGHT,
		DECAY_SHAPE3_LIGHT,
		RELEASE_SHAPE1_LIGHT,
		RELEASE_SHAPE2_LIGHT,
		RELEASE_SHAPE3_LIGHT,
		NUM_LIGHTS
	};

	DADSRHCore _core;

	DADSRHPlus() : TriggerOnLoadModule(
		NUM_PARAMS,
		NUM_INPUTS,
		NUM_OUTPUTS,
		NUM_LIGHTS
	)
	, _core(
		params[DELAY_PARAM],
		params[ATTACK_PARAM],
		params[DECAY_PARAM],
		params[SUSTAIN_PARAM],
		params[RELEASE_PARAM],
		params[HOLD_PARAM],
		params[ATTACK_SHAPE_PARAM],
		params[DECAY_SHAPE_PARAM],
		params[RELEASE_SHAPE_PARAM],
		params[TRIGGER_PARAM],
		params[MODE_PARAM],
		params[LOOP_PARAM],
		params[SPEED_PARAM],
		params[RETRIGGER_PARAM],

		&inputs[DELAY_INPUT],
		&inputs[ATTACK_INPUT],
		&inputs[DECAY_INPUT],
		&inputs[SUSTAIN_INPUT],
		&inputs[RELEASE_INPUT],
		&inputs[HOLD_INPUT],
		inputs[TRIGGER_INPUT],

		&outputs[DELAY_OUTPUT],
		&outputs[ATTACK_OUTPUT],
		&outputs[DECAY_OUTPUT],
		&outputs[SUSTAIN_OUTPUT],
		&outputs[RELEASE_OUTPUT],
		outputs[ENV_OUTPUT],
		outputs[INV_OUTPUT],
		outputs[TRIGGER_OUTPUT],

		lights[DELAY_LIGHT],
		lights[ATTACK_LIGHT],
		lights[DECAY_LIGHT],
		lights[SUSTAIN_LIGHT],
		lights[RELEASE_LIGHT],
		lights[ATTACK_SHAPE1_LIGHT],
		lights[ATTACK_SHAPE2_LIGHT],
		lights[ATTACK_SHAPE3_LIGHT],
		lights[DECAY_SHAPE1_LIGHT],
		lights[DECAY_SHAPE2_LIGHT],
		lights[DECAY_SHAPE3_LIGHT],
		lights[RELEASE_SHAPE1_LIGHT],
		lights[RELEASE_SHAPE2_LIGHT],
		lights[RELEASE_SHAPE3_LIGHT],

		_triggerOnLoad,
		_shouldTriggerOnLoad
	) {
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
