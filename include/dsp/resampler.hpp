#pragma once

#include <assert.h>
#include <string.h>
#include <speex/speex_resampler.h>
#include "frame.hpp"
#include "ringbuffer.hpp"
#include "fir.hpp"


namespace rack {

template<int CHANNELS>
struct SampleRateConverter {
	SpeexResamplerState *st = NULL;
	int channels = CHANNELS;
	int quality = SPEEX_RESAMPLER_QUALITY_DEFAULT;
	int inRate = 44100;
	int outRate = 44100;

	SampleRateConverter() {
		refreshState();
	}
	~SampleRateConverter() {
		if (st) {
			speex_resampler_destroy(st);
		}
	}

	/** Sets the number of channels to actually process. This can be at most CHANNELS. */
	void setChannels(int channels) {
		assert(channels <= CHANNELS);
		if (channels == this->channels)
			return;
		this->channels = channels;
		refreshState();
	}

	/** From 0 (worst, fastest) to 10 (best, slowest) */
	void setQuality(int quality) {
		if (quality == this->quality)
			return;
		this->quality = quality;
		refreshState();
	}

	void setRates(int inRate, int outRate) {
		if (inRate == this->inRate && outRate == this->outRate)
			return;
		this->inRate = inRate;
		this->outRate = outRate;
		refreshState();
	}

	void refreshState() {
		if (st) {
			speex_resampler_destroy(st);
			st = NULL;
		}

		if (channels > 0 && inRate != outRate) {
			int err;
			st = speex_resampler_init(channels, inRate, outRate, quality, &err);
			assert(st);
			assert(err == RESAMPLER_ERR_SUCCESS);

			speex_resampler_set_input_stride(st, CHANNELS);
			speex_resampler_set_output_stride(st, CHANNELS);
		}
	}

	/** `in` and `out` are interlaced with the number of channels */
	void process(const Frame<CHANNELS> *in, int *inFrames, Frame<CHANNELS> *out, int *outFrames) {
		assert(in);
		assert(inFrames);
		assert(out);
		assert(outFrames);
		if (st) {
			// Resample each channel at a time
			spx_uint32_t inLen;
			spx_uint32_t outLen;
			for (int i = 0; i < channels; i++) {
				inLen = *inFrames;
				outLen = *outFrames;
				int err = speex_resampler_process_float(st, i, ((const float*) in) + i, &inLen, ((float*) out) + i, &outLen);
				assert(err == RESAMPLER_ERR_SUCCESS);
			}
			*inFrames = inLen;
			*outFrames = outLen;
		}
		else {
			// Simply copy the buffer without conversion
			int frames = min(*inFrames, *outFrames);
			memcpy(out, in, frames * sizeof(Frame<CHANNELS>));
			*inFrames = frames;
			*outFrames = frames;
		}
	}
};


template<int OVERSAMPLE, int QUALITY>
struct Decimator {
	float inBuffer[OVERSAMPLE*QUALITY];
	float kernel[OVERSAMPLE*QUALITY];
	int inIndex;

	Decimator(float cutoff = 0.9f) {
		boxcarLowpassIR(kernel, OVERSAMPLE*QUALITY, cutoff * 0.5f / OVERSAMPLE);
		blackmanHarrisWindow(kernel, OVERSAMPLE*QUALITY);
		reset();
	}
	void reset() {
		inIndex = 0;
		memset(inBuffer, 0, sizeof(inBuffer));
	}
	/** `in` must be length OVERSAMPLE */
	float process(float *in) {
		// Copy input to buffer
		memcpy(&inBuffer[inIndex], in, OVERSAMPLE*sizeof(float));
		// Advance index
		inIndex += OVERSAMPLE;
		inIndex %= OVERSAMPLE*QUALITY;
		// Perform naive convolution
		float out = 0.f;
		for (int i = 0; i < OVERSAMPLE*QUALITY; i++) {
			int index = inIndex - 1 - i;
			index = (index + OVERSAMPLE*QUALITY) % (OVERSAMPLE*QUALITY);
			out += kernel[i] * inBuffer[index];
		}
		return out;
	}
};


template<int OVERSAMPLE, int QUALITY>
struct Upsampler {
	float inBuffer[QUALITY];
	float kernel[OVERSAMPLE*QUALITY];
	int inIndex;

	Upsampler(float cutoff = 0.9f) {
		boxcarLowpassIR(kernel, OVERSAMPLE*QUALITY, cutoff * 0.5f / OVERSAMPLE);
		blackmanHarrisWindow(kernel, OVERSAMPLE*QUALITY);
		reset();
	}
	void reset() {
		inIndex = 0;
		memset(inBuffer, 0, sizeof(inBuffer));
	}
	/** `out` must be length OVERSAMPLE */
	void process(float in, float *out) {
		// Zero-stuff input buffer
		inBuffer[inIndex] = OVERSAMPLE * in;
		// Advance index
		inIndex++;
		inIndex %= QUALITY;
		// Naively convolve each sample
		// TODO replace with polyphase filter hierarchy
		for (int i = 0; i < OVERSAMPLE; i++) {
			float y = 0.f;
			for (int j = 0; j < QUALITY; j++) {
				int index = inIndex - 1 - j;
				index = (index + QUALITY) % QUALITY;
				int kernelIndex = OVERSAMPLE * j + i;
				y += kernel[kernelIndex] * inBuffer[index];
			}
			out[i] = y;
		}
	}
};


} // namespace rack
