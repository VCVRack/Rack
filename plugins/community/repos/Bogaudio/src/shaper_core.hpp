#pragma once

#include "bogaudio.hpp"

namespace bogaudio {

struct ShaperCore {
	enum Stage {
		STOPPED_STAGE,
		ATTACK_STAGE,
		ON_STAGE,
		DECAY_STAGE,
		OFF_STAGE
	};

	Param& _attackParam;
	Param& _onParam;
	Param& _decayParam;
	Param& _offParam;
	Param& _envParam;
	Param& _signalParam;
	Param& _triggerParam;
	Param& _speedParam;
	Param& _loopParam;

	Input& _signalInput;
	Input& _triggerInput;
	Input* _attackInput;
	Input* _onInput;
	Input* _decayInput;
	Input* _offInput;
	Input* _envInput;
	Input* _signalCVInput;

	Output& _signalOutput;
	Output& _envOutput;
	Output& _invOutput;
	Output& _triggerOutput;
	Output* _attackOutput;
	Output* _onOutput;
	Output* _decayOutput;
	Output* _offOutput;

	Light& _attackLight;
	Light& _onLight;
	Light& _decayLight;
	Light& _offLight;

	bool _firstStep = true;
	bool& _triggerOnLoad;
	bool& _shouldTriggerOnLoad;
	Trigger _trigger;
	PulseGenerator _triggerOuptutPulseGen;
	Stage _stage;
	float _stageProgress;

	ShaperCore(
		Param& attackParam,
		Param& onParam,
		Param& decayParam,
		Param& offParam,
		Param& envParam,
		Param& signalParam,
		Param& triggerParam,
		Param& speedParam,
		Param& loopParam,

		Input& signalInput,
		Input& triggerInput,
		Input* attackInput,
		Input* onInput,
		Input* decayInput,
		Input* offInput,
		Input* envInput,
		Input* signalCVInput,

		Output& signalOutput,
		Output& envOutput,
		Output& invOutput,
		Output& triggerOutput,
		Output* attackOutput,
		Output* onOutput,
		Output* decayOutput,
		Output* offOutput,

		Light& attackLight,
		Light& onLight,
		Light& decayLight,
		Light& offLight,

		bool& triggerOnLoad,
		bool& shouldTriggerOnLoad
	) : _attackParam(attackParam)
	, _onParam(onParam)
	, _decayParam(decayParam)
	, _offParam(offParam)
	, _envParam(envParam)
	, _signalParam(signalParam)
	, _triggerParam(triggerParam)
	, _speedParam(speedParam)
	, _loopParam(loopParam)

	, _signalInput(signalInput)
	, _triggerInput(triggerInput)
	, _attackInput(attackInput)
	, _onInput(onInput)
	, _decayInput(decayInput)
	, _offInput(offInput)
	, _envInput(envInput)
	, _signalCVInput(signalCVInput)

	, _signalOutput(signalOutput)
	, _envOutput(envOutput)
	, _invOutput(invOutput)
	, _triggerOutput(triggerOutput)
	, _attackOutput(attackOutput)
	, _onOutput(onOutput)
	, _decayOutput(decayOutput)
	, _offOutput(offOutput)

	, _attackLight(attackLight)
	, _onLight(onLight)
	, _decayLight(decayLight)
	, _offLight(offLight)

	, _triggerOnLoad(triggerOnLoad)
	, _shouldTriggerOnLoad(shouldTriggerOnLoad)
	{
		reset();
	}

	void reset();
	void step();

	bool stepStage(const Param& knob, const Input* cv, bool slow);
	float levelParam(const Param& knob, const Input* cv) const;
};

} // namespace bogaudio
