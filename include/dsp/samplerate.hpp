#pragma once

#include <assert.h>
#include <string.h>
#include <speex/speex_resampler.h>
#include "frame.hpp"


namespace rack {

template<int CHANNELS>
struct SampleRateConverter {
	SpeexResamplerState *state = NULL;
	bool noConversion = true;
	int inRate = 44100;
	int outRate = 44100;

	SampleRateConverter() {
		int error;
		state = speex_resampler_init(CHANNELS, inRate, outRate, SPEEX_RESAMPLER_QUALITY_DEFAULT, &error);
		assert(error == RESAMPLER_ERR_SUCCESS);
	}
	~SampleRateConverter() {
		speex_resampler_destroy(state);
	}

	void setRates(int in, int out) {
		if (in != inRate || out != outRate) { // speex doesn't optimize setting the rates to the existing values.
			int error = speex_resampler_set_rate(state, in, out);
			assert(error == RESAMPLER_ERR_SUCCESS);
			inRate = in;
			outRate = out;
			noConversion = in == out;
		}
	}

	void setRatioSmooth(float ratio) DEPRECATED {
		// FIXME: this doesn't do a smooth change -- speex doesn't appear to support that.
		const int base = 1000;
		setRates(base, ratio * base);
	}

	/** `in` and `out` are interlaced with the number of channels */
	void process(const Frame<CHANNELS> *in, int *inFrames, Frame<CHANNELS> *out, int *outFrames) {
		if (noConversion) {
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
