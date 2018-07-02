#pragma once

#include "bogaudio.hpp"

namespace bogaudio {

struct DADSRHCore {
	enum Stage {
		STOPPED_STAGE,
		DELAY_STAGE,
		ATTACK_STAGE,
		DECAY_STAGE,
		SUSTAIN_STAGE,
		RELEASE_STAGE
	};

	Param& _delayParam;
	Param& _attackParam;
	Param& _decayParam;
	Param& _sustainParam;
	Param& _releaseParam;
	Param& _holdParam;
	Param& _attackShapeParam;
	Param& _decayShapeParam;
	Param& _releaseShapeParam;
	Param& _triggerParam;
	Param& _modeParam;
	Param& _loopParam;
	Param& _speedParam;
	Param& _retriggerParam;

	Input* _delayInput;
	Input* _attackInput;
	Input* _decayInput;
	Input* _sustainInput;
	Input* _releaseInput;
	Input* _holdInput;
	Input& _triggerInput;

	Output* _delayOutput;
	Output* _attackOutput;
	Output* _decayOutput;
	Output* _sustainOutput;
	Output* _releaseOutput;
	Output& _envOutput;
	Output& _invOutput;
	Output& _triggerOutput;

	Light& _delayLight;
	Light& _attackLight;
	Light& _decayLight;
	Light& _sustainLight;
	Light& _releaseLight;
	Light& _attackShape1Light;
	Light& _attackShape2Light;
	Light& _attackShape3Light;
	Light& _decayShape1Light;
	Light& _decayShape2Light;
	Light& _decayShape3Light;
	Light& _releaseShape1Light;
	Light& _releaseShape2Light;
	Light& _releaseShape3Light;

	bool _firstStep = true;
	bool& _triggerOnLoad;
	bool& _shouldTriggerOnLoad;
	SchmittTrigger _trigger;
	PulseGenerator _triggerOuptutPulseGen;
	Stage _stage;
	float _envelope, _stageProgress, _holdProgress, _releaseLevel;

	DADSRHCore(
		Param& delayParam,
		Param& attackParam,
		Param& decayParam,
		Param& sustainParam,
		Param& releaseParam,
		Param& holdParam,
		Param& attackShapeParam,
		Param& decayShapeParam,
		Param& releaseShapeParam,
		Param& triggerParam,
		Param& modeParam,
		Param& loopParam,
		Param& speedParam,
		Param& retriggerParam,

		Input* delayInput,
		Input* attackInput,
		Input* decayInput,
		Input* sustainInput,
		Input* releaseInput,
		Input* holdInput,
		Input& triggerInput,

		Output* delayOutput,
		Output* attackOutput,
		Output* decayOutput,
		Output* sustainOutput,
		Output* releaseOutput,
		Output& envOutput,
		Output& invOutput,
		Output& triggerOutput,

		Light& delayLight,
		Light& attackLight,
		Light& decayLight,
		Light& sustainLight,
		Light& releaseLight,
		Light& attackShape1Light,
		Light& attackShape2Light,
		Light& attackShape3Light,
		Light& decayShape1Light,
		Light& decayShape2Light,
		Light& decayShape3Light,
		Light& releaseShape1Light,
		Light& releaseShape2Light,
		Light& releaseShape3Light,

		bool& triggerOnLoad,
		bool& shouldTriggerOnLoad
	) : _delayParam(delayParam)
	, _attackParam(attackParam)
	, _decayParam(decayParam)
	, _sustainParam(sustainParam)
	, _releaseParam(releaseParam)
	, _holdParam(holdParam)
	, _attackShapeParam(attackShapeParam)
	, _decayShapeParam(decayShapeParam)
	, _releaseShapeParam(releaseShapeParam)
	, _triggerParam(triggerParam)
	, _modeParam(modeParam)
	, _loopParam(loopParam)
	, _speedParam(speedParam)
	, _retriggerParam(retriggerParam)

	, _delayInput(delayInput)
	, _attackInput(attackInput)
	, _decayInput(decayInput)
	, _sustainInput(sustainInput)
	, _releaseInput(releaseInput)
	, _holdInput(holdInput)
	, _triggerInput(triggerInput)

	, _delayOutput(delayOutput)
	, _attackOutput(attackOutput)
	, _decayOutput(decayOutput)
	, _sustainOutput(sustainOutput)
	, _releaseOutput(releaseOutput)
	, _envOutput(envOutput)
	, _invOutput(invOutput)
	, _triggerOutput(triggerOutput)

	, _delayLight(delayLight)
	, _attackLight(attackLight)
	, _decayLight(decayLight)
	, _sustainLight(sustainLight)
	, _releaseLight(releaseLight)
	, _attackShape1Light(attackShape1Light)
	, _attackShape2Light(attackShape2Light)
	, _attackShape3Light(attackShape3Light)
	, _decayShape1Light(decayShape1Light)
	, _decayShape2Light(decayShape2Light)
	, _decayShape3Light(decayShape3Light)
	, _releaseShape1Light(releaseShape1Light)
	, _releaseShape2Light(releaseShape2Light)
	, _releaseShape3Light(releaseShape3Light)

	, _triggerOnLoad(triggerOnLoad)
	, _shouldTriggerOnLoad(shouldTriggerOnLoad)
	{
		reset();
	}

	void reset();
	void step();

	float stepAmount(const Param& knob, const Input* cv, bool slow, bool allowZero = false);
	float knobTime(const Param& knob, const Input* cv, bool slow, bool allowZero = false);
	float knobAmount(const Param& knob, const Input* cv) const;
};

} // namespace bogaudio
