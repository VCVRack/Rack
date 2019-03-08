#pragma once

#include <math.h>

#include "math.hpp"
#include "table.hpp"

namespace bogaudio {
namespace dsp {

// "amplitude" is 0-whatever here, with 1 (=0db) meaning unity gain.
inline float decibelsToAmplitude(float db) {
	return powf(10.0f, db * 0.05f);
}

inline float amplitudeToDecibels(float amplitude) {
	return 20.0f * log10f(amplitude);
}

struct Amplifier {
	static const float minDecibels;
	static const float maxDecibels;
	static const float decibelsRange;
	struct LevelTable : Table {
		LevelTable(int n) : Table(n) {}
		void _generate() override;
	};
	struct StaticLevelTable : StaticTable<LevelTable, 13> {};

	float _db = 0.0f;
	float _level;
	const Table& _table;

	Amplifier() : _table(StaticLevelTable::table())	{
		setLevel(minDecibels);
	}

	void setLevel(float db);
	float next(float s);
};

struct RunningAverage {
	float _maxDelayMS;
	float _sampleRate = -1.0f;
	float _sensitivity = -1.0f;

	bool _initialized = false;
	float* _buffer = NULL;
	int _bufferN = 0;
	int _sumN = 0;
	int _leadI = 0;
	int _trailI = 0;
	double _sum = 0;

	RunningAverage(float sampleRate = 1000.0f, float sensitivity = 1.0f, float maxDelayMS = 300.0f) : _maxDelayMS(maxDelayMS) {
		setSampleRate(sampleRate);
		setSensitivity(sensitivity);
	}
	virtual ~RunningAverage() {
		if (_buffer) {
			delete[] _buffer;
		}
	}

	void setSampleRate(float sampleRate);
	void setSensitivity(float sensitivity);
	void reset();
	virtual float next(float sample);
};

struct RootMeanSquare : RunningAverage {
	RootMeanSquare(float sampleRate = 1000.0f, float sensitivity = 1.0f, float maxDelayMS = 300.0f)
	: RunningAverage(sampleRate, sensitivity, maxDelayMS)
	{
	}

	float next(float sample) override;
};

// Puckette 2007, "Theory and Technique"
struct PucketteEnvelopeFollower {
	float _sensitivity = -1.0f;
	float _lastOutput = 0.0f;

	PucketteEnvelopeFollower(float sensitivity = 1.0f) {
		setSensitivity(sensitivity);
	}

	void setSensitivity(float sensitivity);
	float next(float sample);
};

struct PositiveZeroCrossing {
	const float positiveThreshold = 0.01f;
	const float negativeThreshold = -positiveThreshold;
	const int zeroesForReset = 20;

	enum State {
		NEGATIVE_STATE,
		POSITIVE_STATE,
		COUNT_ZEROES_STATE
	};

	State _state;
	bool _triggerable;
	int _zeroCount = 0;

	PositiveZeroCrossing(bool triggerable = true)
	: _triggerable(triggerable)
	{
		reset();
	}

	bool next(float sample);
	void reset();
};

struct SlewLimiter {
	float _delta;
	float _last = 0.0f;

	SlewLimiter(float sampleRate = 1000.0f, float milliseconds = 1.0f, float range = 10.0f) {
		setParams(sampleRate, milliseconds, range);
	}

	void setParams(float sampleRate = 1000.0f, float milliseconds = 1.0f, float range = 10.0f);
	inline float next(float sample) {
		return _last = next(sample, _last);
	}
	float next(float sample, float last);
};

struct ShapedSlewLimiter {
	const float range = 10.0f;
	const float minShape = 0.1f;
	const float maxShape = 5.0f;
	float _sampleTime;
	float _time;
	float _shapeExponent;
	float _inverseShapeExponent;
	float _last = 0.0;

	ShapedSlewLimiter(float sampleRate = 1000.0f, float milliseconds = 1.0f, float shape = 1.0f) {
		setParams(sampleRate, milliseconds, shape);
	}

	void setParams(float sampleRate, float milliseconds, float shape);
	float next(float sample);
};

struct CrossFader {
	float _mix = 2.0f;
	float _curve = 1.0f;
	bool _linear = true;
	float _aMix;
	float _bMix;
	Amplifier _aAmp;
	Amplifier _bAmp;

	CrossFader() {
		setParams(0.0f);
	}

	void setParams(
		float mix, // -1 to 1, 0 for equal output of both inputs.
		float curve = 1.0f, // -1 to 1: at -1, A will cut fully as mix goes to 0; at 0, A cuts over full mix; at 1, A cuts from 0 to 1.  B symmetric.
		bool linear = true// cut is linear in amplitude if true; linear in decibels otherwise.
	);
	float next(float a, float b);
};

struct Panner {
	float _pan = 2.0f;
	float _lLevel = 0.0f;
	float _rLevel = 0.0f;
	const Table& _sineTable;

	Panner() : _sineTable(StaticSineTable::table()) {
		setPan(0.0f);
	}

	void setPan(float pan); // -1.0 full left, 0.0 even, 1.0f full right.
	void next(float sample, float& l, float& r);
};

struct DelayLine {
	const float _maxTimeMS;
	float _sampleRate = -1.0f;
	int _bufferN;
	float* _buffer = NULL;
	float _time = -1.0f;
	bool _initialized = false;
	int _delaySamples;
	int _leadI;
	int _trailI;

	DelayLine(float sampleRate = 1000.0f, float maxTimeMS = 1000.0f, float time = 1.0f) : _maxTimeMS(maxTimeMS) {
		setSampleRate(sampleRate);
		setTime(time);
	}
	~DelayLine() {
		delete[] _buffer;
	}

	void setSampleRate(float sampleRate);
	void setTime(float time);
	float next(float sample);
	int delaySamples();
};

struct Limiter {
	float _shape;
	float _knee;
	float _limit;
	float _scale;
	float _normalization;
	FastTanhf _tanhf;

	Limiter() {}

	void setParams(float shape = 1.0f, float knee = 5.0f, float limit = 10.0f, float scale = 2.0f);
	float next(float sample);
};

struct Saturator {
	static const float limit;

	float next(float sample);
};

struct Compressor {
	static const float maxEffectiveRatio;
	float compressionDb(float detectorDb, float thresholdDb, float ratio, bool softKnee);
};

struct NoiseGate {
	static const float maxEffectiveRatio;
	float compressionDb(float detectorDb, float thresholdDb, float ratio, bool softKnee);
};

} // namespace dsp
} // namespace bogaudio
