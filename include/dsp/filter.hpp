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

	/** Sets \f$ \lambda = 1 / \tau \f$. */
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


/** Limits the derivative of the output by a rise and fall speed, in units/s. */
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


/** Behaves like ExponentialFilter but with different lambas when the RHS of the ODE is positive or negative. */
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
	void setRiseFallTau(T riseTau, T fallTau) {
		this->riseLambda = 1 / riseTau;
		this->fallLambda = 1 / fallTau;
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


/** Digital IIR filter processor.
https://en.wikipedia.org/wiki/Infinite_impulse_response
*/
template <int B_ORDER, int A_ORDER, typename T = float>
struct IIRFilter {
	/** transfer function numerator coefficients: b_0, b_1, etc.
	*/
	T b[B_ORDER] = {};
	/** transfer function denominator coefficients: a_1, a_2, etc.
	a_0 is fixed to 1 and omitted from the `a` array, so its indices are shifted down by 1.
	*/
	T a[A_ORDER - 1] = {};
	/** input state
	x[0] = x_{n-1}
	x[1] = x_{n-2}
	etc.
	*/
	T x[B_ORDER - 1];
	/** output state */
	T y[A_ORDER - 1];

	IIRFilter() {
		reset();
	}

	void reset() {
		for (int i = 1; i < B_ORDER; i++) {
			x[i - 1] = 0.f;
		}
		for (int i = 1; i < A_ORDER; i++) {
			y[i - 1] = 0.f;
		}
	}

	void setCoefficients(const T* b, const T* a) {
		for (int i = 0; i < B_ORDER; i++) {
			this->b[i] = b[i];
		}
		for (int i = 1; i < A_ORDER; i++) {
			this->a[i - 1] = a[i - 1];
		}
	}

	T process(T in) {
		T out = 0.f;
		// Add x state
		if (0 < B_ORDER) {
			out = b[0] * in;
		}
		for (int i = 1; i < B_ORDER; i++) {
			out += b[i] * x[i - 1];
		}
		// Subtract y state
		for (int i = 1; i < A_ORDER; i++) {
			out -= a[i - 1] * y[i - 1];
		}
		// Shift x state
		for (int i = B_ORDER - 1; i >= 2; i--) {
			x[i - 1] = x[i - 2];
		}
		x[0] = in;
		// Shift y state
		for (int i = A_ORDER - 1; i >= 2; i--) {
			y[i - 1] = y[i - 2];
		}
		y[0] = out;
		return out;
	}

	/** Computes the complex transfer function $H(s)$ at a particular frequency
	s: normalized angular frequency equal to $2 \pi f / f_{sr}$ ($\pi$ is the Nyquist frequency)
	*/
	std::complex<T> getTransferFunction(T s) {
		// Compute sum(a_k z^-k) / sum(b_k z^-k) where z = e^(i s)
		std::complex<T> bSum(b[0], 0);
		std::complex<T> aSum(1, 0);
		for (int i = 1; i < std::max(B_ORDER, A_ORDER); i++) {
			T p = -i * s;
			std::complex<T> z(simd::cos(p), simd::sin(p));
			if (i < B_ORDER)
				bSum += b[i] * z;
			if (i < A_ORDER)
				aSum += a[i - 1] * z;
		}
		return bSum / aSum;
	}

	T getFrequencyResponse(T f) {
		// T hReal, hImag;
		// getTransferFunction(2 * M_PI * f, &hReal, &hImag);
		// return simd::hypot(hReal, hImag);
		return simd::abs(getTransferFunction(2 * M_PI * f));
	}

	T getFrequencyPhase(T f) {
		return simd::arg(getTransferFunction(2 * M_PI * f));
	}
};


template <typename T = float>
struct TBiquadFilter : IIRFilter<3, 3, T> {
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
		setParameters(LOWPASS, 0.f, 0.f, 1.f);
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
				this->a[0] = -std::exp(-2.f * M_PI * f);
				this->a[1] = 0.f;
				this->b[0] = 1.f + this->a[0];
				this->b[1] = 0.f;
				this->b[2] = 0.f;
			} break;

			case HIGHPASS_1POLE: {
				this->a[0] = std::exp(-2.f * M_PI * (0.5f - f));
				this->a[1] = 0.f;
				this->b[0] = 1.f - this->a[0];
				this->b[1] = 0.f;
				this->b[2] = 0.f;
			} break;

			case LOWPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				this->b[0] = K * K * norm;
				this->b[1] = 2.f * this->b[0];
				this->b[2] = this->b[0];
				this->a[0] = 2.f * (K * K - 1.f) * norm;
				this->a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			case HIGHPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				this->b[0] = norm;
				this->b[1] = -2.f * this->b[0];
				this->b[2] = this->b[0];
				this->a[0] = 2.f * (K * K - 1.f) * norm;
				this->a[1] = (1.f - K / Q + K * K) * norm;

			} break;

			case LOWSHELF: {
				float sqrtV = std::sqrt(V);
				if (V >= 1.f) {
					float norm = 1.f / (1.f + M_SQRT2 * K + K * K);
					this->b[0] = (1.f + M_SQRT2 * sqrtV * K + V * K * K) * norm;
					this->b[1] = 2.f * (V * K * K - 1.f) * norm;
					this->b[2] = (1.f - M_SQRT2 * sqrtV * K + V * K * K) * norm;
					this->a[0] = 2.f * (K * K - 1.f) * norm;
					this->a[1] = (1.f - M_SQRT2 * K + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f + M_SQRT2 / sqrtV * K + K * K / V);
					this->b[0] = (1.f + M_SQRT2 * K + K * K) * norm;
					this->b[1] = 2.f * (K * K - 1) * norm;
					this->b[2] = (1.f - M_SQRT2 * K + K * K) * norm;
					this->a[0] = 2.f * (K * K / V - 1.f) * norm;
					this->a[1] = (1.f - M_SQRT2 / sqrtV * K + K * K / V) * norm;
				}
			} break;

			case HIGHSHELF: {
				float sqrtV = std::sqrt(V);
				if (V >= 1.f) {
					float norm = 1.f / (1.f + M_SQRT2 * K + K * K);
					this->b[0] = (V + M_SQRT2 * sqrtV * K + K * K) * norm;
					this->b[1] = 2.f * (K * K - V) * norm;
					this->b[2] = (V - M_SQRT2 * sqrtV * K + K * K) * norm;
					this->a[0] = 2.f * (K * K - 1.f) * norm;
					this->a[1] = (1.f - M_SQRT2 * K + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f / V + M_SQRT2 / sqrtV * K + K * K);
					this->b[0] = (1.f + M_SQRT2 * K + K * K) * norm;
					this->b[1] = 2.f * (K * K - 1.f) * norm;
					this->b[2] = (1.f - M_SQRT2 * K + K * K) * norm;
					this->a[0] = 2.f * (K * K - 1.f / V) * norm;
					this->a[1] = (1.f / V - M_SQRT2 / sqrtV * K + K * K) * norm;
				}
			} break;

			case BANDPASS: {
				float norm = 1.f / (1.f + K / Q + K * K);
				this->b[0] = K / Q * norm;
				this->b[1] = 0.f;
				this->b[2] = -this->b[0];
				this->a[0] = 2.f * (K * K - 1.f) * norm;
				this->a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			case PEAK: {
				if (V >= 1.f) {
					float norm = 1.f / (1.f + K / Q + K * K);
					this->b[0] = (1.f + K / Q * V + K * K) * norm;
					this->b[1] = 2.f * (K * K - 1.f) * norm;
					this->b[2] = (1.f - K / Q * V + K * K) * norm;
					this->a[0] = this->b[1];
					this->a[1] = (1.f - K / Q + K * K) * norm;
				}
				else {
					float norm = 1.f / (1.f + K / Q / V + K * K);
					this->b[0] = (1.f + K / Q + K * K) * norm;
					this->b[1] = 2.f * (K * K - 1.f) * norm;
					this->b[2] = (1.f - K / Q + K * K) * norm;
					this->a[0] = this->b[1];
					this->a[1] = (1.f - K / Q / V + K * K) * norm;
				}
			} break;

			case NOTCH: {
				float norm = 1.f / (1.f + K / Q + K * K);
				this->b[0] = (1.f + K * K) * norm;
				this->b[1] = 2.f * (K * K - 1.f) * norm;
				this->b[2] = this->b[0];
				this->a[0] = this->b[1];
				this->a[1] = (1.f - K / Q + K * K) * norm;
			} break;

			default: break;
		}
	}
};

typedef TBiquadFilter<> BiquadFilter;


} // namespace dsp
} // namespace rack
