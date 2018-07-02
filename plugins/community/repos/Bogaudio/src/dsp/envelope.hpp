#pragma once

#include <stdlib.h>

#include "base.hpp"

namespace bogaudio {
namespace dsp {

struct EnvelopeGenerator : Generator {
	float _sampleRate;
	float _sampleTime;

	EnvelopeGenerator(float sampleRate = 1000.0f)
	: _sampleRate(sampleRate > 1.0 ? sampleRate : 1.0)
	, _sampleTime(1.0f / _sampleRate)
	{
	}

	void setSampleRate(float sampleRate);
	virtual void _sampleRateChanged() {}
};

struct ADSR : EnvelopeGenerator {
	enum Stage {
		STOPPED_STAGE,
		ATTACK_STAGE,
		DECAY_STAGE,
		SUSTAIN_STAGE,
		RELEASE_STAGE
	};

	Stage _stage = STOPPED_STAGE;
	bool _gated = false;
	float _attack = 0.0f;
	float _decay = 0.0f;
	float _sustain = 1.0f;
	float _release = 0.0f;
	float _attackShape;
	float _decayShape;
	float _releaseShape;
	float _stageProgress = 0.0f;
	float _releaseLevel = 0.0f;
	float _envelope = 0.0f;

	ADSR(bool linear = false, float sampleRate = 1000.0f) : EnvelopeGenerator(sampleRate) {
		setLinearShape(linear);
	}

	void reset();
	void setGate(bool high);
	void setAttack(float seconds);
	void setDecay(float seconds);
	void setSustain(float level);
	void setRelease(float seconds);
	void setLinearShape(bool linear);
	void setShapes(float attackShape, float decayShape, float releaseShape);
	bool isStage(Stage stage) { return _stage == stage; }

	float _next() override;
};

} // namespace dsp
} // namespace bogaudio
