#pragma once

#include <random>

#include "base.hpp"
#include "filter.hpp"

namespace bogaudio {
namespace dsp {

class Seeds {
private:
	std::mt19937 _generator;
	Seeds();
	unsigned int _next();

public:
	Seeds(const Seeds&) = delete;
	void operator=(const Seeds&) = delete;
	static Seeds& getInstance();

	static unsigned int next();
};

struct NoiseGenerator : Generator {
	std::minstd_rand _generator; // one of the faster options.

	NoiseGenerator() : _generator(Seeds::next()) {}
};

struct WhiteNoiseGenerator : NoiseGenerator {
	std::uniform_real_distribution<float> _uniform;

	WhiteNoiseGenerator() : _uniform(-1.0, 1.0) {}

	float _next() override {
		return _uniform(_generator);
	}
};

template<typename G>
struct BasePinkNoiseGenerator : NoiseGenerator {
	static const int _n = 7;
	G _g;
	G _gs[_n];
	uint32_t _count = _g.next();

	float _next() override {
		// See: http://www.firstpr.com.au/dsp/pink-noise/
		float sum = _g.next();
		for (int i = 0, bit = 1; i < _n; ++i, bit <<= 1) {
			if (_count & bit) {
				sum += _gs[i].next();
			}
			else {
				sum += _gs[i].current();
			}
		}
		++_count;
		return sum / (float)(_n + 1);
	}
};

struct PinkNoiseGenerator : BasePinkNoiseGenerator<WhiteNoiseGenerator> {};

struct RedNoiseGenerator : BasePinkNoiseGenerator<PinkNoiseGenerator> {};

struct BlueNoiseGenerator : NoiseGenerator {
	PinkNoiseGenerator _pink;
	float _last = 0.0f;

	float _next() override {
		float t = _last;
		_last = _pink.next();
		return _last - t;
	}
};

struct GaussianNoiseGenerator : NoiseGenerator {
	std::normal_distribution<float> _normal;

	GaussianNoiseGenerator(float mean = 0.0f, float stdDev = 1.0f) : _normal(mean, stdDev) {}

	float _next() override {
		return _normal(_generator);
	}
};

struct RandomWalk : Generator {
	float _min;
	float _max;
	float _last = 0.0f;
	float _lastOut = 0.0f;
	float _damp;
	float _bias = 0.0f;
	float _biasDamp = 1.0f;
	WhiteNoiseGenerator _noise;
	LowPassFilter _filter;

	RandomWalk(
		float min = -5.0f,
		float max = 5.0f,
		float sampleRate = 1000.0f,
		float change = 0.5f
	)
	: _min(min)
	, _max(max)
	{
		setParams(sampleRate, change);
	}

	void setParams(float sampleRate = 1000.0f, float change = 0.5f);
	void jump();
	void tell(float v);
	float _next() override;
};

} // namespace dsp
} // namespace bogaudio
