#pragma once

#include "dsp/fir.hpp"


namespace rack {

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

} // namespace rack
