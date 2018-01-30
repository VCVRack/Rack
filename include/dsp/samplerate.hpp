#pragma once

#include <assert.h>
#include <string.h>
#include <speex/speex_resampler.h>
#include "frame.hpp"


namespace rack {

template<int CHANNELS>
struct SampleRateConverter {
	SpeexResamplerState *state;
	bool bypass = false;

	SampleRateConverter() {
		int error;
		state = speex_resampler_init(CHANNELS, 44100, 44100, SPEEX_RESAMPLER_QUALITY_DEFAULT, &error);
		assert(error == RESAMPLER_ERR_SUCCESS);
	}
	~SampleRateConverter() {
		speex_resampler_destroy(state);
	}

	void setQuality(int quality) {
		speex_resampler_set_quality(state, quality);
	}

	void setRates(int inRate, int outRate) {
		int oldInRate, oldOutRate;
		getRates(&oldInRate, &oldOutRate);
		if (inRate == oldInRate && outRate == oldOutRate)
			return;
		int error = speex_resampler_set_rate(state, inRate, outRate);
		assert(error == RESAMPLER_ERR_SUCCESS);
	}

	void getRates(int *inRate, int *outRate) {
		spx_uint32_t inRate32, outRate32;
		speex_resampler_get_rate(state, &inRate32, &outRate32);
		if (inRate) *inRate = inRate32;
		if (outRate) *outRate = outRate32;
	}

	/** `in` and `out` are interlaced with the number of channels */
	void process(const Frame<CHANNELS> *in, int *inFrames, Frame<CHANNELS> *out, int *outFrames) {
		if (bypass) {
			int len = std::min(*inFrames, *outFrames);
			memcpy(out, in, len * sizeof(Frame<CHANNELS>));
			*inFrames = len;
			*outFrames = len;
			return;
		}
		speex_resampler_process_interleaved_float(state, (const float*)in, (unsigned int*)inFrames, (float*)out, (unsigned int*)outFrames);
	}

	void reset() {
		int error = speex_resampler_reset_mem(state);
		assert(error == RESAMPLER_ERR_SUCCESS);
	}
};

} // namespace rack
