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

	void setTau(T tau) {
		this->lambda = 1 / tau;
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

	void setTau(T tau) {
		this->lambda = 1 / tau;
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


template <typename T = float>
struct TBiquadFilter {
	/** input state */
	T x[2];
	/** output state */
	T y[2];

	/** transfer function numerator coefficients: b_0, b_1, b_2 */
	float b[3];
	/** transfer function denominator coefficients: a_1, a_2
	a_0 is fixed to 1.
	*/
	float a[2];

	enum Type {
		LOWPASS_1POLE,
		HIGHPASS_1POLE,
		LOWPASS,
		HIGHPASS,
		LOWSHELF,
		HIGHSHELF,
		BANDPASS,
		PEAK,
		NOTCH,
		NUM_TYPES
	};

	TBiquadFilter() {
		reset();
		setParameters(LOWPASS, 0.f, 0.f, 1.f);
	}

	void reset() {
		std::memset(x, 0, sizeof(x));
		std::memset(y, 0, sizeof(y));
	}

	T process(T in) {
		// Advance IIR
		T out = b[0] * in + b[1] * x[0] + b[2] * x[1]
		        - a[0] * y[0] - a[1] * y[1];
		// Push input
		x[1] = x[0];
		x[0] = in;
		// Push output
		y[1] = y[0];
		y[0] = out;
		return out;
	}

	/** Calculates and sets the biquad transfer function coefficients.
	f: normalized frequency (cutoff frequency / sample rate), must be less than 0.5
	Q: quality factor
	V: gain
	*/
	void setParameters(Type type, float f, float Q, float V) {
		float K = std::tan(M_PI * f);
		switch (type) {
			case LOWPASS_1POLE: {
				a[0] = -std::exp(-2.f * M_PI * f);
				a[1] = 0.f;
				b[0] = 1.f + a[0];
				b[1] = 0.f;
				b[2] = 0.f;
			} break;

			case HIGHPASS_1POLE: {
				a[0] = std::exp(-2.f * M_PI * (0.5f - f));
				a[1] = 0.f;
				b[0] = 1.f - a[0];
				b[1] = 0.f;
				b[2] = 0.f;
			} break;

			case LOWPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				b[0] = K * K * norm;
				b[1] = 2.f * b[0];
				b[2] = b[0];
				a[0] = 2.f * (K * K - 1.f) * norm;
				a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			case HIGHPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				b[0] = norm;
				b[1] = -2.f * b[0];
				b[2] = b[0];
				a[0] = 2.f * (K * K - 1.f) * norm;
				a[1] = (1.f - K / Q + K * K) * norm;

			} break;

			case LOWSHELF: {
				float sqrtV = std::sqrt(V);
				if (V >= 1.f) {
					float norm = 1.f / (1.f + M_SQRT2 * K + K * K);
					b[0] = (1.f + M_SQRT2 * sqrtV * K + V * K * K) * norm;
					b[1] = 2.f * (V * K * K - 1.f) * norm;
					b[2] = (1.f - M_SQRT2 * sqrtV * K + V * K * K) * norm;
					a[0] = 2.f * (K * K - 1.f) * norm;
					a[1] = (1.f - M_SQRT2 * K + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f + M_SQRT2 / sqrtV * K + K * K / V);
					b[0] = (1.f + M_SQRT2 * K + K * K) * norm;
					b[1] = 2.f * (K * K - 1) * norm;
					b[2] = (1.f - M_SQRT2 * K + K * K) * norm;
					a[0] = 2.f * (K * K / V - 1.f) * norm;
					a[1] = (1.f - M_SQRT2 / sqrtV * K + K * K / V) * norm;
				}
			} break;

			case HIGHSHELF: {
				float sqrtV = std::sqrt(V);
				if (V >= 1.f) {
					float norm = 1.f / (1.f + M_SQRT2 * K + K * K);
					b[0] = (V + M_SQRT2 * sqrtV * K + K * K) * norm;
					b[1] = 2.f * (K * K - V) * norm;
					b[2] = (V - M_SQRT2 * sqrtV * K + K * K) * norm;
					a[0] = 2.f * (K * K - 1.f) * norm;
					a[1] = (1.f - M_SQRT2 * K + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f / V + M_SQRT2 / sqrtV * K + K * K);
					b[0] = (1.f + M_SQRT2 * K + K * K) * norm;
					b[1] = 2.f * (K * K - 1.f) * norm;
					b[2] = (1.f - M_SQRT2 * K + K * K) * norm;
					a[0] = 2.f * (K * K - 1.f / V) * norm;
					a[1] = (1.f / V - M_SQRT2 / sqrtV * K + K * K) * norm;
				}
			} break;

			case BANDPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				b[0] = K / Q * norm;
				b[1] = 0.f;
				b[2] = -b[0];
				a[0] = 2.f * (K * K - 1.f) * norm;
				a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			case PEAK: {
				if (V >= 1.f) {
					float norm = 1.f / (1.f + K / Q + K * K);
					b[0] = (1.f + K / Q * V + K * K) * norm;
					b[1] = 2.f * (K * K - 1.f) * norm;
					b[2] = (1.f - K / Q * V + K * K) * norm;
					a[0] = b[1];
					a[1] = (1.f - K / Q + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f + K / Q / V + K * K);
					b[0] = (1.f + K / Q + K * K) * norm;
					b[1] = 2.f * (K * K - 1.f) * norm;
					b[2] = (1.f - K / Q + K * K) * norm;
					a[0] = b[1];
					a[1] = (1.f - K / Q / V + K * K) * norm;
				}
			} break;

			case NOTCH: {
				float norm = 1.f / (1.f + K / Q + K * K);
				b[0] = (1.f + K * K) * norm;
				b[1] = 2.f * (K * K - 1.f) * norm;
				b[2] = b[0];
				a[0] = b[1];
				a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			default: break;
		}
	}

	void copyParameters(const TBiquadFilter<T>& from) {
		b[0] = from.b[0];
		b[1] = from.b[1];
		b[2] = from.b[2];
		a[0] = from.a[0];
		a[1] = from.a[1];
	}

	/** Computes the gain of a particular frequency
	f: normalized frequency
	*/
	float getFrequencyResponse(float f) {
		// Compute sum(a_k e^(-i k f))
		std::complex<float> bsum = b[0];
		std::complex<float> asum = 1.f;
		for (int i = 1; i < 3; i++) {
			float p = 2 * M_PI * -i * f;
			std::complex<float> e(std::cos(p), std::sin(p));
			bsum += b[i] * e;
			asum += a[i - 1] * e;
		}
		return std::abs(bsum / asum);
	}
};

typedef TBiquadFilter<> BiquadFilter;


} // namespace dsp
} // namespace rack
