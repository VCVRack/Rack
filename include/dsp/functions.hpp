#pragma once

#include "util/math.hpp"


namespace rack {


/** The normalized sinc function. https://en.wikipedia.org/wiki/Sinc_function */
inline float sinc(float x) {
	if (x == 0.f)
		return 1.f;
	x *= M_PI;
	return std::sin(x) / x;
}

inline float quadraticBipolar(float x) {
	float x2 = x*x;
	return (x >= 0.f) ? x2 : -x2;
}

inline float cubic(float x) {
	return x*x*x;
}

inline float quarticBipolar(float x) {
	float y = x*x*x*x;
	return (x >= 0.f) ? y : -y;
}

inline float quintic(float x) {
	// optimal with --fast-math
	return x*x*x*x*x;
}

inline float sqrtBipolar(float x) {
	return (x >= 0.f) ? std::sqrt(x) : -std::sqrt(-x);
}

/** This is pretty much a scaled sinh */
inline float exponentialBipolar(float b, float x) {
	const float a = b - 1.f / b;
	return (std::pow(b, x) - std::pow(b, -x)) / a;
}

inline float gainToDb(float gain) {
	return std::log10(gain) * 20.f;
}

inline float dbToGain(float db) {
	return std::pow(10.f, db / 20.f);
}


} // namespace rack
