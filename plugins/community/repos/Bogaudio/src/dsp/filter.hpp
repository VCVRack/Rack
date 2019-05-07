#pragma once

#include <stdint.h>
#include <math.h>

#include "buffer.hpp"

namespace bogaudio {
namespace dsp {

struct Filter {
	Filter() {}
	virtual ~Filter() {}

	virtual float next(float sample) = 0;
};

template<typename T>
struct BiquadFilter : Filter {
	T _a0 = 0.0;
	T _a1 = 0.0;
	T _a2 = 0.0;
	T _b1 = 0.0;
	T _b2 = 0.0 ;

	T _x[3] {};
	T _y[3] {};

	BiquadFilter() {}

	void setParams(T a0, T a1, T a2, T b0, T b1, T b2) {
		if (b0 == 1.0) {
			_a0 = a0;
			_a1 = a1;
			_a2 = a2;
			_b1 = b1;
			_b2 = b2;
		}
		else {
			_a0 = a0 / b0;
			_a1 = a1 / b0;
			_a2 = a2 / b0;
			_b1 = b1 / b0;
			_b2 = b2 / b0;
		}
	}

	void reset() {
		_x[0] = _x[1] = _x[2] = 0.0;
		_y[0] = _y[1] = _y[2] = 0.0;
	}

	float next(float sample) override {
		_x[2] = _x[1];
		_x[1] = _x[0];
		_x[0] = sample;

		_y[2] = _y[1];
		_y[1] = _y[0];
		_y[0] = _a0 * _x[0];
		_y[0] += _a1 * _x[1];
		_y[0] += _a2 * _x[2];
		_y[0] -= _b1 * _y[1];
		_y[0] -= _b2 * _y[2];

		return _y[0];
	}
};

struct ComplexBiquadFilter : BiquadFilter<float> {
	float _gain = 1.0f;
	float _zeroRadius = 1.0f;
	float _zeroTheta = M_PI;
	float _poleRadius = 0.9f;
	float _poleTheta = 0.0f;

	ComplexBiquadFilter() {
		updateParams();
	}

	void setComplexParams(
		float gain,
		float zeroRadius,
		float zeroTheta,
		float poleRadius,
		float poleTheta
	);
	void updateParams();
};

struct LowPassFilter : Filter {
	float _sampleRate = 0.0f;
	float _cutoff = 0.0f;
	float _q = 0.0f;

	BiquadFilter<float> _biquad;

	LowPassFilter(float sampleRate = 1000.0f, float cutoff = 100.0f, float q = 0.001f) {
		setParams(sampleRate, cutoff, q);
	}

	void setParams(float sampleRate, float cutoff, float q = 0.001f);
	void reset() { _biquad.reset(); }
	float next(float sample) override {
		return _biquad.next(sample);
	}
};

struct MultipoleFilter : Filter {
	enum Type {
		LP_TYPE,
		HP_TYPE
	};

	static constexpr int maxPoles = 20;
	static constexpr float maxRipple = 0.29f;
	Type _type = LP_TYPE;
	int _poles = 1;
	float _sampleRate = 0.0f;
	float _cutoff = 0.0f;
	float _ripple = 0.0f;
	BiquadFilter<double> _biquads[maxPoles / 2];

	MultipoleFilter() {}

	void setParams(
		Type type,
		int poles,
		float sampleRate,
		float cutoff,
		float ripple // FIXME: using this with more than two poles creates large gain, need compensation.
	);
	float next(float sample) override;
};

struct Decimator {
	Decimator() {}
	virtual ~Decimator() {}

	virtual void setParams(float sampleRate, int factor) = 0;
	virtual float next(const float* buf) = 0;
};

struct LPFDecimator : Decimator {
	int _factor;
	MultipoleFilter _filter;

	LPFDecimator(float sampleRate = 1000.0f, int factor = 8) {
		setParams(sampleRate, factor);
	}
	void setParams(float sampleRate, int factor) override;
	float next(const float* buf) override;
};

struct CICDecimator : Decimator {
	typedef int64_t T;
	static constexpr T scale = ((T)1) << 32;
	int _stages;
	T* _integrators;
	T* _combs;
	int _factor = 0;
	float _gainCorrection;

	CICDecimator(int stages = 4, int factor = 8);
	virtual ~CICDecimator();

	void setParams(float sampleRate, int factor) override;
	float next(const float* buf) override;
};

struct Interpolator {
	Interpolator() {}
	virtual ~Interpolator() {}

	virtual void setParams(float sampleRate, int factor) = 0;
	virtual void next(float sample, float* buf) = 0;
};

struct CICInterpolator : Interpolator {
	typedef int64_t T;
	static constexpr T scale = ((T)1) << 32;
	int _stages;
	T* _integrators;
	T* _combs;
	T* _buffer;
	int _factor = 0;
	float _gainCorrection;

	CICInterpolator(int stages = 4, int factor = 8);
	virtual ~CICInterpolator();

	void setParams(float sampleRate, int factor) override;
	void next(float sample, float* buf) override;
};

} // namespace dsp
} // namespace bogaudio
