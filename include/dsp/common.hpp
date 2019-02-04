#pragma once
#include "math.hpp"


namespace rack {
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

// Window functions

/** Hann window function
p: proportion from [0, 1], usually `i / (len - 1)`
https://en.wikipedia.org/wiki/Window_function#Hann_and_Hamming_windows
*/
inline float hann(float p) {
	return 0.5f * (1.f - std::cos(2*M_PI * p));
}

/** Applies the Hann window to a signal `x` */
inline void hannWindow(float *x, int len) {
	for (int i = 0; i < len; i++) {
		x[i] *= hann((float) i / (len - 1));
	}
}

/** Blackman window function
https://en.wikipedia.org/wiki/Window_function#Blackman_window
A typical alpha value is 0.16.
*/
inline float blackman(float alpha, float p) {
	return
		+ (1 - alpha) / 2.f
		- 1 / 2.f * std::cos(2*M_PI * p)
		+ alpha / 2.f * std::cos(4*M_PI * p);
}

inline void blackmanWindow(float alpha, float *x, int len) {
	for (int i = 0; i < len; i++) {
		x[i] *= blackman(alpha, (float) i / (len - 1));
	}
}


/** Blackman-Nuttall window function
https://en.wikipedia.org/wiki/Window_function#Blackman%E2%80%93Nuttall_window
*/
inline float blackmanNuttall(float p) {
	return
		+ 0.3635819f
		- 0.4891775f * std::cos(2*M_PI * p)
		+ 0.1365995f * std::cos(4*M_PI * p)
		- 0.0106411f * std::cos(6*M_PI * p);
}

inline void blackmanNuttallWindow(float *x, int len) {
	for (int i = 0; i < len; i++) {
		x[i] *= blackmanNuttall((float) i / (len - 1));
	}
}

/** Blackman-Harris window function
https://en.wikipedia.org/wiki/Window_function#Blackman%E2%80%93Harris_window
*/
inline float blackmanHarris(float p) {
	return
		+ 0.35875f
		- 0.48829f * std::cos(2*M_PI * p)
		+ 0.14128f * std::cos(4*M_PI * p)
		- 0.01168f * std::cos(6*M_PI * p);
}

inline void blackmanHarrisWindow(float *x, int len) {
	for (int i = 0; i < len; i++) {
		x[i] *= blackmanHarris((float) i / (len - 1));
	}
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
