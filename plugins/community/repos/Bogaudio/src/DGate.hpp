#pragma once

#include "bogaudio.hpp"

extern Model* modelDGate;

namespace bogaudio {

struct DGate : TriggerOnLoadModule {
	enum ParamsIds {
		DELAY_PARAM,
		GATE_PARAM,
		LOOP_PARAM,
		TRIGGER_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		TRIGGER_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		GATE_OUTPUT,
		END_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		DELAY_LIGHT,
		GATE_LIGHT,
		NUM_LIGHTS
	};

	enum Stage {
		STOPPED_STAGE,
		DELAY_STAGE,
		GATE_STAGE
	};

	bool _firstStep = true;
	SchmittTrigger _trigger;
	PulseGenerator _triggerOuptutPulseGen;
	Stage _stage;
	float _stageProgress;

	DGate() : TriggerOnLoadModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}

	void onReset() override;
	void step() override;
	bool stepStage(Param& knob);
	bool shouldTriggerOnNextLoad() override {
		return _stage != STOPPED_STAGE;
	};
};

} // namespace bogaudio
