#pragma once

#include <assert.h>
#include <samplerate.h>
#include "frame.hpp"


namespace rack {

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

} // namespace rack
