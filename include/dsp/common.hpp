#pragma once
#include "math.hpp"


namespace rack {


/** Digital signal processing routines for plugins
*/
namespace dsp {


// Constants

static const float FREQ_C4 = 261.6256f;
static const float FREQ_A4 = 440.0000f;

// Mathematical functions

/** The normalized sinc function
See https://en.wikipedia.org/wiki/Sinc_function
*/
inline float sinc(float x) {
	if (x == 0.f)
		return 1.f;
	x *= M_PI;
	return std::sin(x) / x;
}

// Conversion functions

inline float amplitudeToDb(float amp) {
	return std::log10(amp) * 20.f;
}

inline float dbToAmplitude(float db) {
	return std::pow(10.f, db / 20.f);
}

// Functions for parameter scaling

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
	// optimal with -fassociative-math
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


} // namespace dsp
} // namespace rack
