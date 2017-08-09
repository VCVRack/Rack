#pragma once

#include <assert.h>
#include <string.h>
#include <samplerate.h>
#include <complex>
#include "math.hpp"
#include "../ext/dr_libs/dr_wav.h"


namespace rack {


/** Useful for storing arrays of samples in ring buffers and casting them to `float*` to be used by interleaved processors, like SampleRateConverter */
template <size_t CHANNELS>
struct Frame {
	float samples[CHANNELS];
};


/** Simple FFT implementation
If you need something fast, use pffft, KissFFT, etc instead.
The size N must be a power of 2
*/
struct SimpleFFT {
	int N;
	/** Twiddle factors e^(2pi k/N), interleaved complex numbers */
	std::complex<float> *tw;
	SimpleFFT(int N, bool inverse) : N(N) {
		tw = new std::complex<float>[N];
		for (int i = 0; i < N; i++) {
			float phase = 2*M_PI * (float)i / N;
			if (inverse)
				phase *= -1.0;
			tw[i] = std::exp(std::complex<float>(0.0, phase));
		}
	}
	~SimpleFFT() {
		delete[] tw;
	}
	/** Reference naive implementation
	x and y are arrays of interleaved complex numbers
	y must be size N/s
	s is the stride factor for the x array which divides the size N
	*/
	void dft(const std::complex<float> *x, std::complex<float> *y, int s=1) {
		for (int k = 0; k < N/s; k++) {
			std::complex<float> yk = 0.0;
			for (int n = 0; n < N; n += s) {
				int m = (n*k) % N;
				yk += x[n] * tw[m];
			}
			y[k] = yk;
		}
	}
	void fft(const std::complex<float> *x, std::complex<float> *y, int s=1) {
		if (N/s <= 2) {
			// Naive DFT is faster than further FFT recursions at this point
			dft(x, y, s);
			return;
		}
		std::complex<float> *e = new std::complex<float>[N/(2*s)]; // Even inputs
		std::complex<float> *o = new std::complex<float>[N/(2*s)]; // Odd inputs
		fft(x, e, 2*s);
		fft(x + s, o, 2*s);
		for (int k = 0; k < N/(2*s); k++) {
			int m = (k*s) % N;
			y[k] = e[k] + tw[m] * o[k];
			y[k + N/(2*s)] = e[k] - tw[m] * o[k];
		}
		delete[] e;
		delete[] o;
	}
};


typedef void (*stepCallback)(float x, const float y[], float dydt[]);
/** Solve an ODE system using the 1st order Euler method */
inline void stepEuler(stepCallback f, float x, float dx, float y[], int len) {
	float k[len];

	f(x, y, k);
	for (int i = 0; i < len; i++) {
		y[i] += dx * k[i];
	}
}
/** Solve an ODE system using the 4th order Runge-Kutta method */
inline void stepRK4(stepCallback f, float x, float dx, float y[], int len) {
	float k1[len];
	float k2[len];
	float k3[len];
	float k4[len];
	float yi[len];

	f(x, y, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k1[i] * dx / 2.0;
	}
	f(x + dx / 2.0, yi, k2);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k2[i] * dx / 2.0;
	}
	f(x + dx / 2.0, yi, k3);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k3[i] * dx;
	}
	f(x + dx, yi, k4);

	for (int i = 0; i < len; i++) {
		y[i] += dx * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) / 6.0;
	}
}


/** A simple cyclic buffer.
S must be a power of 2.
push() is constant time O(1)
*/
template <typename T, int S>
struct RingBuffer {
	T data[S];
	int start = 0;
	int end = 0;

	int mask(int i) const {
		return i & (S - 1);
	}
	void push(T t) {
		int i = mask(end++);
		data[i] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	void clear() {
		start = end;
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	int size() const {
		return end - start;
	}
	int capacity() const {
		return S - size();
	}
};


/** A cyclic buffer which maintains a valid linear array of size S by keeping a copy of the buffer in adjacent memory.
S must be a power of 2.
push() is constant time O(2) relative to RingBuffer
*/
template <typename T, int S>
struct DoubleRingBuffer {
	T data[S*2];
	int start = 0;
	int end = 0;

	int mask(int i) const {
		return i & (S - 1);
	}
	void push(T t) {
		int i = mask(end++);
		data[i] = t;
		data[i + S] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	void clear() {
		start = end;
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	int size() const {
		return end - start;
	}
	int capacity() const {
		return S - size();
	}
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T *endData() {
		return &data[mask(end)];
	}
	void endIncr(int n) {
		int e = mask(end);
		int e1 = e + n;
		int e2 = mini(e1, S);
		// Copy data forward
		memcpy(data + S + e, data + e, sizeof(T) * (e2 - e));

		if (e1 > S) {
			// Copy data backward from the doubled block to the main block
			memcpy(data, data + S, sizeof(T) * (e1 - S));
		}
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[mask(start)];
	}
	void startIncr(int n) {
		start += n;
	}
};


/** A cyclic buffer which maintains a valid linear array of size S by sliding along a larger block of size N.
The linear array of S elements are moved back to the start of the block once it outgrows past the end.
This happens every N - S pushes, so the push() time is O(1 + S / (N - S)).
For example, a float buffer of size 64 in a block of size 1024 is nearly as efficient as RingBuffer.
*/
template <typename T, size_t S, size_t N>
struct AppleRingBuffer {
	T data[N];
	size_t start = 0;
	size_t end = 0;

	void push(T t) {
		data[end++] = t;
		if (end >= N) {
			// move end block to beginning
			memmove(data, &data[N - S], sizeof(T) * S);
			start -= N - S;
			end = S;
		}
	}
	T shift() {
		return data[start++];
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	size_t size() const {
		return end - start;
	}
	/** Returns a pointer to S consecutive elements for appending, requesting to append n elements.
	*/
	T *endData(size_t n) {
		// TODO
		return &data[end];
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[start];
	}
	void startIncr(size_t n) {
		// This is valid as long as n < S
		start += n;
	}
};


template<int CHANNELS>
struct SampleRateConverter {
	SRC_STATE *state;
	SRC_DATA data;

	SampleRateConverter() {
		int error;
		state = src_new(SRC_SINC_FASTEST, CHANNELS, &error);
		assert(!error);

		data.src_ratio = 1.0;
		data.end_of_input = false;
	}
	~SampleRateConverter() {
		src_delete(state);
	}
	/** output_sample_rate / input_sample_rate */
	void setRatio(float r) {
		src_set_ratio(state, r);
		data.src_ratio = r;
	}
	void setRatioSmooth(float r) {
		data.src_ratio = r;
	}
	/** `in` and `out` are interlaced with the number of channels */
	void process(const Frame<CHANNELS> *in, int *inFrames, Frame<CHANNELS> *out, int *outFrames) {
		// Old versions of libsamplerate use float* here instead of const float*
		data.data_in = (float*) in;
		data.input_frames = *inFrames;
		data.data_out = (float*) out;
		data.output_frames = *outFrames;
		src_process(state, &data);
		*inFrames = data.input_frames_used;
		*outFrames = data.output_frames_gen;
	}
	void reset() {
		src_reset(state);
	}
};


/** Perform a direct convolution
x[-len + 1] to x[0] must be defined
*/
inline float convolve(const float *x, const float *kernel, int len) {
	float y = 0.0;
	for (int i = 0; i < len; i++) {
		y += x[-i] * kernel[i];
	}
	return y;
}

inline void blackmanHarrisWindow(float *x, int n) {
	const float a0 = 0.35875;
	const float a1 = 0.48829;
	const float a2 = 0.14128;
	const float a3 = 0.01168;
	for (int i = 0; i < n; i++) {
		x[i] *= a0
			- a1 * cosf(2 * M_PI * i / (n - 1))
			+ a2 * cosf(4 * M_PI * i / (n - 1))
			- a3 * cosf(6 * M_PI * i / (n - 1));
	}
}

inline void boxcarFIR(float *x, int n, float cutoff) {
	for (int i = 0; i < n; i++) {
		float t = (float)i / (n - 1) * 2.0 - 1.0;
		x[i] = sincf(t * n * cutoff);
	}
}


template<int OVERSAMPLE, int QUALITY>
struct Decimator {
	DoubleRingBuffer<float, OVERSAMPLE*QUALITY> inBuffer;
	float kernel[OVERSAMPLE*QUALITY];

	Decimator(float cutoff = 0.9) {
		boxcarFIR(kernel, OVERSAMPLE*QUALITY, cutoff * 0.5 / OVERSAMPLE);
		blackmanHarrisWindow(kernel, OVERSAMPLE*QUALITY);
		// The sum of the kernel should be 1
		float sum = 0.0;
		for (int i = 0; i < OVERSAMPLE*QUALITY; i++) {
			sum += kernel[i];
		}
		for (int i = 0; i < OVERSAMPLE*QUALITY; i++) {
			kernel[i] /= sum;
		}
	}
	float process(float *in) {
		memcpy(inBuffer.endData(), in, OVERSAMPLE*sizeof(float));
		inBuffer.endIncr(OVERSAMPLE);
		float out = convolve(inBuffer.endData() + OVERSAMPLE*QUALITY, kernel, OVERSAMPLE*QUALITY);
		// Ignore the ring buffer's start position
		return out;
	}
};


// Pre-made minBLEP samples in minBLEP.cpp
extern const float minblep_16_32[];


template<int ZERO_CROSSINGS>
struct MinBLEP {
	float buf[2*ZERO_CROSSINGS] = {};
	int pos = 0;
	const float *minblep;
	int oversample;

	/** Places a discontinuity with magnitude dx at -1 < p <= 0 relative to the current frame */
	void jump(float p, float dx) {
		if (p <= -1 || 0 < p)
			return;
		for (int j = 0; j < 2*ZERO_CROSSINGS; j++) {
			float minblepIndex = ((float)j - p) * oversample;
			int index = (pos + j) % (2*ZERO_CROSSINGS);
			buf[index] += dx * (-1.0 + interpf(minblep, minblepIndex));
		}
	}
	float shift() {
		float v = buf[pos];
		buf[pos] = 0.0;
		pos = (pos + 1) % (2*ZERO_CROSSINGS);
		return v;
	}
};


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


struct SchmittTrigger {
	/** 0 unknown, 1 low, 2 high */
	int state = 0;
	float low = 0.0;
	float high = 1.0;
	void setThresholds(float low, float high) {
		this->low = low;
		this->high = high;
	}
	/** Returns true if triggered */
	bool process(float in) {
		bool triggered = false;
		if (in >= high) {
			if (state == 1)
				triggered = true;
			state = 2;
		}
		else if (in <= low) {
			state = 1;
		}
		return triggered;
	}
	void reset() {
		state = 0;
	}
};


} // namespace rack
