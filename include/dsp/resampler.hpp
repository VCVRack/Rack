#pragma once
#include <speex/speex_resampler.h>

#include <dsp/common.hpp>
#include <dsp/ringbuffer.hpp>
#include <dsp/fir.hpp>
#include <dsp/window.hpp>


namespace rack {
namespace dsp {


/** Resamples by a fixed rational factor. */
template <int MAX_CHANNELS>
struct SampleRateConverter {
	SpeexResamplerState* st = NULL;
	int channels = MAX_CHANNELS;
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

	/** Sets the number of channels to actually process. This can be at most MAX_CHANNELS. */
	void setChannels(int channels) {
		assert(channels <= MAX_CHANNELS);
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
			(void) err;
		}
	}

	void process(const float* in, int inStride, int* inFrames, float* out, int outStride, int* outFrames) {
		assert(in);
		assert(inFrames);
		assert(out);
		assert(outFrames);

		if (st) {
			speex_resampler_set_input_stride(st, inStride);
			speex_resampler_set_output_stride(st, outStride);
			// Resample each channel at a time
			spx_uint32_t inLen = 0;
			spx_uint32_t outLen = 0;
			for (int c = 0; c < channels; c++) {
				inLen = *inFrames;
				outLen = *outFrames;
				int err = speex_resampler_process_float(st, c, &in[c], &inLen, &out[c], &outLen);
				(void) err;
			}
			*inFrames = inLen;
			*outFrames = outLen;
		}
		else {
			// Simply copy the buffer without conversion
			int frames = std::min(*inFrames, *outFrames);
			for (int i = 0; i < frames; i++) {
				for (int c = 0; c < channels; c++) {
					out[outStride * i + c] = in[inStride * i + c];
				}
			}
			*inFrames = frames;
			*outFrames = frames;
		}
	}

	void process(const Frame<MAX_CHANNELS>* in, int* inFrames, Frame<MAX_CHANNELS>* out, int* outFrames) {
		process((const float*) in, MAX_CHANNELS, inFrames, (float*) out, MAX_CHANNELS, outFrames);
	}
};


/** Downsamples by an integer factor. */
template <int OVERSAMPLE, int QUALITY, typename T = float>
struct Decimator {
	T inBuffer[OVERSAMPLE * QUALITY];
	float kernel[OVERSAMPLE * QUALITY];
	int inIndex;

	Decimator(float cutoff = 0.9f) {
		boxcarLowpassIR(kernel, OVERSAMPLE * QUALITY, cutoff * 0.5f / OVERSAMPLE);
		blackmanHarrisWindow(kernel, OVERSAMPLE * QUALITY);
		reset();
	}
	void reset() {
		inIndex = 0;
		std::memset(inBuffer, 0, sizeof(inBuffer));
	}
	/** `in` must be length OVERSAMPLE */
	T process(T* in) {
		// Copy input to buffer
		std::memcpy(&inBuffer[inIndex], in, OVERSAMPLE * sizeof(T));
		// Advance index
		inIndex += OVERSAMPLE;
		inIndex %= OVERSAMPLE * QUALITY;
		// Perform naive convolution
		T out = 0.f;
		for (int i = 0; i < OVERSAMPLE * QUALITY; i++) {
			int index = inIndex - 1 - i;
			index = (index + OVERSAMPLE * QUALITY) % (OVERSAMPLE * QUALITY);
			out += kernel[i] * inBuffer[index];
		}
		return out;
	}
};


/** Upsamples by an integer factor. */
template <int OVERSAMPLE, int QUALITY>
struct Upsampler {
	float inBuffer[QUALITY];
	float kernel[OVERSAMPLE * QUALITY];
	int inIndex;

	Upsampler(float cutoff = 0.9f) {
		boxcarLowpassIR(kernel, OVERSAMPLE * QUALITY, cutoff * 0.5f / OVERSAMPLE);
		blackmanHarrisWindow(kernel, OVERSAMPLE * QUALITY);
		reset();
	}
	void reset() {
		inIndex = 0;
		std::memset(inBuffer, 0, sizeof(inBuffer));
	}
	/** `out` must be length OVERSAMPLE */
	void process(float in, float* out) {
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


} // namespace dsp
} // namespace rack
