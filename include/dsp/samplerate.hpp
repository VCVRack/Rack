#pragma once

#include <assert.h>
#include <string.h>
#include <speex/speex_resampler.h>
#include "frame.hpp"


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

} // namespace rack
