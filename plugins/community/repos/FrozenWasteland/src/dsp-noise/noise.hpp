#pragma once

#include <random>

#include "base.hpp"

namespace frozenwasteland {
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

	virtual float _next() override {
		return _uniform(_generator);
	}
};

template<typename G>
struct BasePinkNoiseGenerator : NoiseGenerator {
	static const int _n = 7;
	G _g;
	G _gs[_n];
	uint32_t _count = _g.next();

	virtual float _next() override {
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

struct GaussianNoiseGenerator : NoiseGenerator {
	std::normal_distribution<float> _normal;

	GaussianNoiseGenerator() : _normal(0, 1.0) {}

	virtual float _next() override {
		return _normal(_generator);
	}
};

} // namespace dsp
} // namespace frozenwasteland
