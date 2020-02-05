#pragma once
#include <pffft.h>

#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** Real-valued FFT context.
Wrapper for [PFFFT](https://bitbucket.org/jpommier/pffft/)
`length` must be a multiple of 32.
Buffers must be aligned to 16-byte boundaries. new[] and malloc() do this for you.
*/
struct RealFFT {
	PFFFT_Setup* setup;
	int length;

	RealFFT(size_t length) {
		this->length = length;
		setup = pffft_new_setup(length, PFFFT_REAL);
	}

	~RealFFT() {
		pffft_destroy_setup(setup);
	}

	/** Performs the real FFT.
	Input and output must be aligned using the above align*() functions.
	Input is `length` elements. Output is `2*length` elements.
	Output is arbitrarily ordered for performance reasons.
	However, this ordering is consistent, so element-wise multiplication with line up with other results, and the inverse FFT will return a correctly ordered result.
	*/
	void rfftUnordered(const float* input, float* output) {
		pffft_transform(setup, input, output, NULL, PFFFT_FORWARD);
	}

	/** Performs the inverse real FFT.
	Input is `2*length` elements. Output is `length` elements.
	Scaling is such that IRFFT(RFFT(x)) = N*x.
	*/
	void irfftUnordered(const float* input, float* output) {
		pffft_transform(setup, input, output, NULL, PFFFT_BACKWARD);
	}

	/** Slower than the above methods, but returns results in the "canonical" FFT order as follows.
		output[0] = F(0)
		output[1] = F(n/2)
		output[2] = real(F(1))
		output[3] = imag(F(1))
		output[4] = real(F(2))
		output[5] = imag(F(2))
		...
		output[length - 2] = real(F(n/2 - 1))
		output[length - 1] = imag(F(n/2 - 1))
	*/
	void rfft(const float* input, float* output) {
		pffft_transform_ordered(setup, input, output, NULL, PFFFT_FORWARD);
	}

	void irfft(const float* input, float* output) {
		pffft_transform_ordered(setup, input, output, NULL, PFFFT_BACKWARD);
	}

	/** Scales the RFFT so that `scale(IFFT(FFT(x))) = x`.
	*/
	void scale(float* x) {
		float a = 1.f / length;
		for (int i = 0; i < length; i++) {
			x[i] *= a;
		}
	}
};


/** Complex-valued FFT context.
`length` must be a multiple of 16.
*/
struct ComplexFFT {
	PFFFT_Setup* setup;
	int length;

	ComplexFFT(size_t length) {
		this->length = length;
		setup = pffft_new_setup(length, PFFFT_COMPLEX);
	}

	~ComplexFFT() {
		pffft_destroy_setup(setup);
	}

	/** Performs the complex FFT.
	Input and output must be aligned using the above align*() functions.
	Input is `2*length` elements. Output is `2*length` elements.
	*/
	void fftUnordered(const float* input, float* output) {
		pffft_transform(setup, input, output, NULL, PFFFT_FORWARD);
	}

	/** Performs the inverse complex FFT.
	Input is `2*length` elements. Output is `2*length` elements.
	Scaling is such that FFT(IFFT(x)) = N*x.
	*/
	void ifftUnordered(const float* input, float* output) {
		pffft_transform(setup, input, output, NULL, PFFFT_BACKWARD);
	}

	void fft(const float* input, float* output) {
		pffft_transform_ordered(setup, input, output, NULL, PFFFT_FORWARD);
	}

	void ifft(const float* input, float* output) {
		pffft_transform_ordered(setup, input, output, NULL, PFFFT_BACKWARD);
	}

	void scale(float* x) {
		float a = 1.f / length;
		for (int i = 0; i < length; i++) {
			x[2 * i + 0] *= a;
			x[2 * i + 1] *= a;
		}
	}
};


} // namespace dsp
} // namespace rack
