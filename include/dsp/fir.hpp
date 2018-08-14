#pragma once
#include "dsp/functions.hpp"
#include "pffft.h"


namespace rack {

/** Performs a direct sum convolution */
inline float convolveNaive(const float *in, const float *kernel, int len) {
	float y = 0.f;
	for (int i = 0; i < len; i++) {
		y += in[len - 1 - i] * kernel[i];
	}
	return y;
}

/** Computes the impulse response of a boxcar lowpass filter */
inline void boxcarLowpassIR(float *out, int len, float cutoff = 0.5f) {
	for (int i = 0; i < len; i++) {
		float t = i - (len - 1) / 2.f;
		out[i] = 2 * cutoff * sinc(2 * cutoff * t);
	}
}

inline void blackmanHarrisWindow(float *x, int len) {
	// Constants from https://en.wikipedia.org/wiki/Window_function#Blackman%E2%80%93Harris_window
	const float a0 = 0.35875f;
	const float a1 = 0.48829f;
	const float a2 = 0.14128f;
	const float a3 = 0.01168f;
	float factor = 2*M_PI / (len - 1);
	for (int i = 0; i < len; i++) {
		x[i] *=
			+ a0
			- a1 * cosf(1*factor * i)
			+ a2 * cosf(2*factor * i)
			- a3 * cosf(3*factor * i);
	}
}


struct RealTimeConvolver {
	// `kernelBlocks` number of contiguous FFT blocks of size `blockSize`
	// indexed by [i * blockSize*2 + j]
	float *kernelFfts = NULL;
	float *inputFfts = NULL;
	float *outputTail = NULL;
	float *tmpBlock = NULL;
	size_t blockSize;
	size_t kernelBlocks = 0;
	size_t inputPos = 0;
	PFFFT_Setup *pffft;

	/** `blockSize` is the size of each FFT block. It should be >=32 and a power of 2. */
	RealTimeConvolver(size_t blockSize) {
		this->blockSize = blockSize;
		pffft = pffft_new_setup(blockSize*2, PFFFT_REAL);
		outputTail = new float[blockSize];
		memset(outputTail, 0, blockSize * sizeof(float));
		tmpBlock = new float[blockSize*2];
		memset(tmpBlock, 0, blockSize*2 * sizeof(float));
	}

	~RealTimeConvolver() {
		setKernel(NULL, 0);
		delete[] outputTail;
		delete[] tmpBlock;
		pffft_destroy_setup(pffft);
	}

	void setKernel(const float *kernel, size_t length) {
		// Clear existing kernel
		if (kernelFfts) {
			pffft_aligned_free(kernelFfts);
			kernelFfts = NULL;
		}
		if (inputFfts) {
			pffft_aligned_free(inputFfts);
			inputFfts = NULL;
		}
		kernelBlocks = 0;
		inputPos = 0;

		if (kernel && length > 0) {
			// Round up to the nearest factor of `blockSize`
			kernelBlocks = (length - 1) / blockSize + 1;

			// Allocate blocks
			kernelFfts = (float*) pffft_aligned_malloc(sizeof(float) * blockSize*2 * kernelBlocks);
			inputFfts = (float*) pffft_aligned_malloc(sizeof(float) * blockSize*2 * kernelBlocks);
			memset(inputFfts, 0, sizeof(float) * blockSize*2 * kernelBlocks);

			for (size_t i = 0; i < kernelBlocks; i++) {
				// Pad each block with zeros
				memset(tmpBlock, 0, sizeof(float) * blockSize*2);
				size_t len = min((int) blockSize, (int) (length - i*blockSize));
				memcpy(tmpBlock, &kernel[i*blockSize], sizeof(float)*len);
				// Compute fft
				pffft_transform(pffft, tmpBlock, &kernelFfts[blockSize*2 * i], NULL, PFFFT_FORWARD);
			}
		}
	}

	/** Applies reverb to input
	input and output must be of size `blockSize`
	*/
	void processBlock(const float *input, float *output) {
		if (kernelBlocks == 0) {
			memset(output, 0, sizeof(float) * blockSize);
			return;
		}

		// Step input position
		inputPos = (inputPos + 1) % kernelBlocks;
		// Pad block with zeros
		memset(tmpBlock, 0, sizeof(float) * blockSize*2);
		memcpy(tmpBlock, input, sizeof(float) * blockSize);
		// Compute input fft
		pffft_transform(pffft, tmpBlock, &inputFfts[blockSize*2 * inputPos], NULL, PFFFT_FORWARD);
		// Create output fft
		memset(tmpBlock, 0, sizeof(float) * blockSize*2);
		// convolve input fft by kernel fft
		// Note: This is the CPU bottleneck loop
		for (size_t i = 0; i < kernelBlocks; i++) {
			size_t pos = (inputPos - i + kernelBlocks) % kernelBlocks;
			pffft_zconvolve_accumulate(pffft, &kernelFfts[blockSize*2 * i], &inputFfts[blockSize*2 * pos], tmpBlock, 1.f);
		}
		// Compute output
		pffft_transform(pffft, tmpBlock, tmpBlock, NULL, PFFFT_BACKWARD);
		// Add block tail from last output block
		for (size_t i = 0; i < blockSize; i++) {
			tmpBlock[i] += outputTail[i];
		}
		// Copy output block to output
		float scale = 1.f / (blockSize*2);
		for (size_t i = 0; i < blockSize; i++) {
			// Scale based on FFT
			output[i] = tmpBlock[i] * scale;
		}
		// Set tail
		for (size_t i = 0; i < blockSize; i++) {
			outputTail[i] = tmpBlock[i + blockSize];
		}
	}
};


} // namespace rack
