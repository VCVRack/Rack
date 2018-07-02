#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>

#include "envelope.hpp"

using namespace bogaudio::dsp;

void EnvelopeGenerator::setSampleRate(float sampleRate) {
	if (_sampleRate != sampleRate && sampleRate >= 1.0) {
		_sampleRate = sampleRate;
		_sampleTime = 1.0f / sampleRate;
		_sampleRateChanged();
	}
}

void ADSR::reset() {
	_stage = STOPPED_STAGE;
	_gated = false;
	_envelope = 0.0f;
}

void ADSR::setGate(bool high) {
	_gated = high;
}

void ADSR::setAttack(float seconds) {
	assert(_attack >= 0.0f);
	_attack = std::max(seconds, 0.001f);
}

void ADSR::setDecay(float seconds) {
	assert(_decay >= 0.0f);
	_decay = std::max(seconds, 0.001f);
}

void ADSR::setSustain(float level) {
	assert(_sustain >= 0.0f);
	assert(_sustain <= 1.0f);
	_sustain = level;
}

void ADSR::setRelease(float seconds) {
	assert(_release >= 0.0f);
	_release = std::max(seconds, 0.001f);
}

void ADSR::setLinearShape(bool linear) {
	if (linear) {
		setShapes(1.0f, 1.0f, 1.0f);
	}
	else {
		setShapes(0.5f, 0.5f, 0.5f);
	}
}

void ADSR::setShapes(float attackShape, float decayShape, float releaseShape) {
	assert(attackShape >= 0.1f && attackShape <= 10.0f);
	assert(decayShape >= 0.0f && decayShape <= 10.0f);
	assert(releaseShape >= 0.0f && releaseShape <= 10.0f);
	_attackShape = attackShape;
	_decayShape = decayShape;
	_releaseShape = releaseShape;
}

float ADSR::_next() {
	if (_gated) {
		switch (_stage) {
			case STOPPED_STAGE: {
				_stage = ATTACK_STAGE;
				_stageProgress = 0.0f;
				break;
			}
			case ATTACK_STAGE: {
				if (_envelope >= 1.0) {
					_stage = DECAY_STAGE;
					_stageProgress = 0.0f;
				}
				break;
			}
			case DECAY_STAGE: {
				if (_stageProgress >= _decay) {
					_stage = SUSTAIN_STAGE;
					_stageProgress = 0.0f;
				}
				break;
			}
			case SUSTAIN_STAGE: {
				break;
			}
			case RELEASE_STAGE: {
				_stage = ATTACK_STAGE;
				_stageProgress = _attack * powf(_envelope, 1.0f / _releaseShape);
				break;
			}
		}
	}
	else {
		switch (_stage) {
			case STOPPED_STAGE: {
				break;
			}
			case ATTACK_STAGE:
			case DECAY_STAGE:
			case SUSTAIN_STAGE: {
				_stage = RELEASE_STAGE;
				_stageProgress = 0.0f;
				_releaseLevel = _envelope;
				break;
			}
			case RELEASE_STAGE: {
				if (_stageProgress >= _release) {
					_stage = STOPPED_STAGE;
				}
				break;
			}
		}
	}

	switch (_stage) {
		case STOPPED_STAGE: {
			_envelope = 0.0f;
			break;
		}
		case ATTACK_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _attack;
			_envelope = powf(_envelope, _attackShape);
			break;
		}
		case DECAY_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _decay;
			_envelope = powf(_envelope, _decayShape);
			_envelope *= 1.0f - _sustain;
			_envelope = 1.0f - _envelope;
			break;
		}
		case SUSTAIN_STAGE: {
			_envelope = _sustain;
			break;
		}
		case RELEASE_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _release;
			_envelope = powf(_envelope, _releaseShape);
			_envelope *= _releaseLevel;
			_envelope = _releaseLevel - _envelope;
			break;
		}
	}

	return _envelope;
}
