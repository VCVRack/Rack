#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <vector>

#include "base.hpp"
#include "math.hpp"
#include "table.hpp"

namespace bogaudio {
namespace dsp {

struct Oscillator {
	float _sampleRate;
	float _frequency;

	Oscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: _sampleRate(sampleRate > 1.0 ? sampleRate : 1.0)
	, _frequency(frequency)
	{
	}
	virtual ~Oscillator() {}

	void setSampleRate(float sampleRate) {
		if (_sampleRate != sampleRate && sampleRate >= 1.0) {
			_sampleRate = sampleRate;
			_sampleRateChanged();
		}
	}

	virtual void _sampleRateChanged() {}

	void setFrequency(float frequency) {
		if (_frequency != frequency) {
			_frequency = frequency;
			_frequencyChanged();
		}
	}

	virtual void _frequencyChanged() {}
};

struct OscillatorGenerator : Oscillator, Generator {
	OscillatorGenerator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: Oscillator(sampleRate, frequency)
	{
	}
};

struct Phasor : OscillatorGenerator {
	typedef uint32_t phase_t;
	typedef int64_t phase_delta_t;
	static constexpr phase_t maxPhase = UINT32_MAX;
	static constexpr float twoPI = 2.0f * M_PI;
	static constexpr float maxSampleWidth = 0.25f;

	phase_delta_t _delta;
	phase_t _phase = 0;
	float _sampleWidth = 0.0f;
	phase_t _samplePhase = 0;

	Phasor(
		float sampleRate = 1000.0f,
		float frequency = 100.0f,
		float initialPhase = 0.0f
	)
	: OscillatorGenerator(sampleRate, frequency)
	{
		setPhase(initialPhase);
		_update();
	}

	void _sampleRateChanged() override {
		_update();
	}

	void _frequencyChanged() override {
		_update();
	}

	void setSampleWidth(float sw);
	void resetPhase();
	void setPhase(float radians);
	float nextFromPhasor(const Phasor& phasor, phase_delta_t offset = 0);
	inline float nextForPhase(phase_t phase) { return _nextForPhase(phase); }
	virtual void _update();
	inline void advancePhase() { _phase += _delta; }
	inline void advancePhase(int n) { assert(n > 0); _phase += n * _delta; }
	float _next() override final;
	virtual float _nextForPhase(phase_t phase);

	inline static phase_delta_t radiansToPhase(float radians) { return (radians / twoPI) * maxPhase; }
	inline static float phaseToRadians(phase_t phase) { return (phase / (float)maxPhase) * twoPI; }
};

struct TablePhasor : Phasor {
	const Table& _table;
	int _tableLength;

	TablePhasor(
		const Table& table,
		double sampleRate = 1000.0f,
		double frequency = 100.0f
	)
	: Phasor(sampleRate, frequency)
	, _table(table)
	, _tableLength(table.length())
	{
	}

	float _nextForPhase(phase_t phase) override;
};

struct SineOscillator : OscillatorGenerator {
	double _k1, _k2;
	double _x;
	double _y;

	SineOscillator(
		double sampleRate = 1000.0f,
		double frequency = 100.0f,
		double initialPhase = 0.0
	)
	: OscillatorGenerator(sampleRate, frequency)
	{
		setPhase(initialPhase);
		update();
	}

	void _sampleRateChanged() override {
		update();
	}

	void _frequencyChanged() override {
		update();
	}

	void setPhase(double phase);
	void update();
	float _next() override;
};

struct SineTableOscillator : TablePhasor {
	SineTableOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: TablePhasor(StaticSineTable::table(), sampleRate, frequency)
	{
	}
};

struct SawOscillator : Phasor {
	SawOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: Phasor(sampleRate, frequency)
	{
	}

	float _nextForPhase(phase_t phase) override;
};

struct SaturatingSawOscillator : SawOscillator {
	float _saturation;
	float _saturationNormalization;
	FastTanhf _tanhf;

	SaturatingSawOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: SawOscillator(sampleRate, frequency)
	, _saturation(0.0f)
	, _saturationNormalization(1.0f)
	{
	}

	void setSaturation(float saturation);

	float _nextForPhase(phase_t phase) override;
};

struct BandLimitedSawOscillator : SaturatingSawOscillator {
	int _quality;
	const Table& _table;
	phase_delta_t _qd = 0;
	float _halfTableLen;

	BandLimitedSawOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f,
		int quality = 5,
		const Table& table = StaticBlepTable::table()
	)
	: SaturatingSawOscillator(sampleRate, frequency)
	, _quality(quality)
	, _table(table)
	, _halfTableLen(_table.length() / 2)
	{
		setQuality(quality);
	}

	void setQuality(int quality);

	void _update() override;
	float _nextForPhase(phase_t phase) override;
};

struct SquareOscillator : Phasor {
	const float minPulseWidth = 0.03f;
	const float maxPulseWidth = 1.0f - minPulseWidth;
	static constexpr float defaultPulseWidth = 0.5f;
	float _pulseWidthInput;
	phase_t _pulseWidth = maxPhase * defaultPulseWidth;
	bool positive = true;

	SquareOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: Phasor(sampleRate, frequency)
	{
	}

	void setPulseWidth(float pw);

	float _nextForPhase(phase_t phase) override;
};

struct BandLimitedSquareOscillator : BandLimitedSawOscillator {
	const float minPulseWidth = 0.03f;
	const float maxPulseWidth = 1.0f - minPulseWidth;
	float _pulseWidthInput;
	phase_delta_t _pulseWidth;
	float _offset;

	BandLimitedSquareOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f,
		int quality = 5,
		const Table& table = StaticBlepTable::table()
	)
	: BandLimitedSawOscillator(sampleRate, frequency, quality, table)
	{
		setPulseWidth(0.5f);
	}

	void setPulseWidth(float pw);

	float _nextForPhase(phase_t phase) override;
};

struct TriangleOscillator : Phasor {
	const phase_t quarterMaxPhase = maxPhase * 0.25f;
	const phase_t threeQuartersMaxPhase = maxPhase * 0.75f;

	TriangleOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f
	)
	: Phasor(sampleRate, frequency)
	{
	}

	float _nextForPhase(phase_t phase) override;
};

struct SineBankOscillator : Oscillator {
	struct Partial {
		float frequency;
		float frequencyRatio;
		float amplitude;
		float amplitudeTarget;
		float amplitudeStepDelta;
		int amplitudeSteps;
		SineTableOscillator sine;

		Partial()
		: frequency(0.0)
		, frequencyRatio(0.0)
		, amplitude(0.0)
		, amplitudeTarget(0.0)
		, amplitudeStepDelta(0.0)
		, amplitudeSteps(0)
		, sine(0.0, 0.0)
		{}
	};

	const float _maxPartialFrequencySRRatio = 0.48;
	float _maxPartialFrequency = 0.0;
	const int _amplitudeEnvelopeMS = 10;
	int _amplitudeEnvelopeSamples = 0;
	std::vector<Partial> _partials;

	SineBankOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f,
		int partialCount = 20
	)
	: Oscillator(sampleRate, frequency)
	, _partials(partialCount)
	{
		_sampleRateChanged();
		_frequencyChanged();
	}

	int partialCount() {
		return _partials.size();
	}

	// one-based indexes.
	void setPartial(int i, float frequencyRatio, float amplitude);
	bool setPartialFrequencyRatio(int i, float frequencyRatio);
	void setPartialAmplitude(int i, float amplitude, bool envelope = false);

	void syncToPhase(float phase);

	void _sampleRateChanged() override;
	void _frequencyChanged() override;
	float next(Phasor::phase_t phaseOffset = 0.0f);
};

} // namespace dsp
} // namespace bogaudio
