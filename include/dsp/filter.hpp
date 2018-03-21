#pragma once

#include "util/math.hpp"


namespace rack {

struct RCFilter {
	float c = 0.f;
	float xstate[1] = {};
	float ystate[1] = {};

	// `r` is the ratio between the cutoff frequency and sample rate, i.e. r = f_c / f_s
	void setCutoff(float r) {
		c = 2.f / r;
	}
	void process(float x) {
		float y = (x + xstate[0] - ystate[0] * (1 - c)) / (1 + c);
		xstate[0] = x;
		ystate[0] = y;
	}
	float lowpass() {
		return ystate[0];
	}
	float highpass() {
		return xstate[0] - ystate[0];
	}
};


struct PeakFilter {
	float state = 0.f;
	float c = 0.f;

	/** Rate is lambda / sampleRate */
	void setRate(float r) {
		c = 1.f - r;
	}
	void process(float x) {
		if (x > state)
			state = x;
		state *= c;
	}
	float peak() {
		return state;
	}
};


struct SlewLimiter {
	float rise = 1.f;
	float fall = 1.f;
	float out = 0.f;

	void setRiseFall(float rise, float fall) {
		this->rise = rise;
		this->fall = fall;
	}
	float process(float in) {
		out = clamp(in, out - fall, out + rise);
		return out;
	}
};


/** Applies exponential smoothing to a signal with the ODE
dy/dt = x * lambda
*/
struct ExponentialFilter {
	float out = 0.f;
	float lambda = 1.f;

	float process(float in) {
		float y = out + (in - out) * lambda;
		// If no change was detected, assume float granularity is too small and snap output to input
		if (out == y)
			out = in;
		else
			out = y;
		return out;
	}
};


} // namespace rack
