#pragma once

#include "math.hpp"


namespace rack {

struct RCFilter {
	float c = 0.0;
	float xstate[1] = {};
	float ystate[1] = {};

	// `r` is the ratio between the cutoff frequency and sample rate, i.e. r = f_c / f_s
	void setCutoff(float r) {
		c = 2.0 / r;
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
	float state = 0.0;
	float c = 0.0;

	/** Rate is lambda / sampleRate */
	void setRate(float r) {
		c = 1.0 - r;
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
	float rise = 1.0;
	float fall = 1.0;
	float out = 0.0;
	float process(float in) {
		float delta = clampf(in - out, -fall, rise);
		out += delta;
		return out;
	}
};


} // namespace rack
