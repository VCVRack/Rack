#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


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
	float rise = 0.f;
	float fall = 0.f;
	float out = NAN;

	float process(float deltaTime, float in) {
		if (std::isnan(out)) {
			out = in;
		}
		else if (out < in) {
			float y = out + rise * deltaTime;
			out = std::fmin(y, in);
		}
		else if (out > in) {
			float y = out - fall * deltaTime;
			out = std::fmax(y, in);
		}
		return out;
	}
	DEPRECATED float process(float in) {
		return process(1.f, in);
	}
	DEPRECATED void setRiseFall(float rise, float fall) {
		this->rise = rise;
		this->fall = fall;
	}
};


struct ExponentialSlewLimiter {
	float riseLambda = 0.f;
	float fallLambda = 0.f;
	float out = NAN;

	float process(float deltaTime, float in) {
		if (std::isnan(out)) {
			out = in;
		}
		else if (out < in) {
			float y = out + (in - out) * riseLambda * deltaTime;
			out = (out == y) ? in : y;
		}
		else if (out > in) {
			float y = out + (in - out) * fallLambda * deltaTime;
			out = (out == y) ? in : y;
		}
		return out;
	}
	DEPRECATED float process(float in) {
		return process(1.f, in);
	}
};


/** Applies exponential smoothing to a signal with the ODE
\f$ \frac{dy}{dt} = x \lambda \f$.
*/
struct ExponentialFilter {
	float out = 0.f;
	float lambda = 0.f;

	void reset() {
		out = 0.f;
	}

	float process(float deltaTime, float in) {
		float y = out + (in - out) * lambda * deltaTime;
		// If no change was made between the old and new output, assume float granularity is too small and snap output to input
		if (out == y)
			out = in;
		else
			out = y;
		return out;
	}

	DEPRECATED float process(float in) {
		return process(1.f, in);
	}
};


} // namespace dsp
} // namespace rack
