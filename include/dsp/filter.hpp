#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** The simplest possible analog filter using an Euler solver.
https://en.wikipedia.org/wiki/RC_circuit
Use two RC filters in series for a bandpass filter.
*/
template <typename T = float>
struct TRCFilter {
	T c = 0.f;
	T xstate[1];
	T ystate[1];

	TRCFilter() {
		reset();
	}

	void reset() {
		xstate[0] = 0.f;
		ystate[0] = 0.f;
	}

	/** Sets the cutoff angular frequency in radians.
	*/
	void setCutoff(T r) {
		c = 2.f / r;
	}
	/** Sets the cutoff frequency.
	`f` is the ratio between the cutoff frequency and sample rate, i.e. f = f_c / f_s
	*/
	void setCutoffFreq(T f) {
		setCutoff(2.f * M_PI * f);
	}
	void process(T x) {
		T y = (x + xstate[0] - ystate[0] * (1 - c)) / (1 + c);
		xstate[0] = x;
		ystate[0] = y;
	}
	T lowpass() {
		return ystate[0];
	}
	T highpass() {
		return xstate[0] - ystate[0];
	}
};

typedef TRCFilter<> RCFilter;


/** Applies exponential smoothing to a signal with the ODE
\f$ \frac{dy}{dt} = x \lambda \f$.
*/
template <typename T = float>
struct TExponentialFilter {
	T out = 0.f;
	T lambda = 0.f;

	void reset() {
		out = 0.f;
	}

	void setLambda(T lambda) {
		this->lambda = lambda;
	}

	T process(T deltaTime, T in) {
		T y = out + (in - out) * lambda * deltaTime;
		// If no change was made between the old and new output, assume T granularity is too small and snap output to input
		out = simd::ifelse(out == y, in, y);
		return out;
	}

	DEPRECATED T process(T in) {
		return process(1.f, in);
	}
};

typedef TExponentialFilter<> ExponentialFilter;


/** Like ExponentialFilter but jumps immediately to higher values.
*/
template <typename T = float>
struct TPeakFilter {
	T out = 0.f;
	T lambda = 0.f;

	void reset() {
		out = 0.f;
	}

	void setLambda(T lambda) {
		this->lambda = lambda;
	}

	T process(T deltaTime, T in) {
		T y = out + (in - out) * lambda * deltaTime;
		out = simd::fmax(y, in);
		return out;
	}
	/** Use the return value of process() instead. */
	DEPRECATED T peak() {
		return out;
	}
	/** Use setLambda() instead. */
	DEPRECATED void setRate(T r) {
		lambda = 1.f - r;
	}
	DEPRECATED T process(T x) {
		return process(1.f, x);
	}
};

typedef TPeakFilter<> PeakFilter;


template <typename T = float>
struct TSlewLimiter {
	T out = 0.f;
	T rise = 0.f;
	T fall = 0.f;

	void reset() {
		out = 0.f;
	}

	void setRiseFall(T rise, T fall) {
		this->rise = rise;
		this->fall = fall;
	}
	T process(T deltaTime, T in) {
		out = simd::clamp(in, out - fall * deltaTime, out + rise * deltaTime);
		return out;
	}
	DEPRECATED T process(T in) {
		return process(1.f, in);
	}
};

typedef TSlewLimiter<> SlewLimiter;


template <typename T = float>
struct TExponentialSlewLimiter {
	T out = 0.f;
	T riseLambda = 0.f;
	T fallLambda = 0.f;

	void reset() {
		out = 0.f;
	}

	void setRiseFall(T riseLambda, T fallLambda) {
		this->riseLambda = riseLambda;
		this->fallLambda = fallLambda;
	}
	T process(T deltaTime, T in) {
		T lambda = simd::ifelse(in > out, riseLambda, fallLambda);
		T y = out + (in - out) * lambda * deltaTime;
		// If the change from the old out to the new out is too small for floats, set `in` directly.
		out = simd::ifelse(out == y, in, y);
		return out;
	}
	DEPRECATED T process(T in) {
		return process(1.f, in);
	}
};

typedef TExponentialSlewLimiter<> ExponentialSlewLimiter;


} // namespace dsp
} // namespace rack
