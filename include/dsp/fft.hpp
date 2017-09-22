#pragma once

#include <complex>


namespace rack {

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

} // namespace rack
