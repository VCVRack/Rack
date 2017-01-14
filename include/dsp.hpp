#pragma once

#include <assert.h>
#include <string.h>
#include <samplerate.h>


namespace rack {


/** S must be a power of 2 */
template <typename T, size_t S>
struct RingBuffer {
	T data[S] = {};
	size_t start = 0;
	size_t end = 0;

	size_t mask(size_t i) {
		return i & (S - 1);
	}
	void push(T t) {
		size_t i = mask(end++);
		data[i] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	bool empty() {
		return start >= end;
	}
	bool full() {
		return end - start >= S;
	}
	size_t size() {
		return end - start;
	}
};


/** S must be a power of 2 */
template <typename T, size_t S>
struct DoubleRingBuffer {
	T data[S*2] = {};
	size_t start = 0;
	size_t end = 0;

	size_t mask(size_t i) {
		return i & (S - 1);
	}
	void push(T t) {
		size_t i = mask(end++);
		data[i] = t;
		data[i + S] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	bool empty() {
		return start >= end;
	}
	bool full() {
		return end - start >= S;
	}
	size_t size() {
		return end - start;
	}
};


template <typename T, size_t S, size_t N>
struct AppleRingBuffer {
	T data[N] = {};
	size_t start = 0;
	size_t end = 0;

	void push(T t) {
		data[end++] = t;
		if (end >= N) {
			// move end block to beginning
			memmove(data, &data[N - S], sizeof(T) * S);
			start -= N - S;
			end = S;
		}
	}
	T shift() {
		return data[start++];
	}
	bool empty() {
		return start >= end;
	}
	bool full() {
		return end - start >= S;
	}
	size_t size() {
		return end - start;
	}
};


template <size_t S>
struct SampleRateConverter {
	SRC_STATE *state;
	SRC_DATA data;

	SampleRateConverter() {
		int error;
		state = src_new(SRC_SINC_FASTEST, 1, &error);
		assert(!error);

		data.src_ratio = 1.0;
		data.end_of_input = false;
	}
	~SampleRateConverter() {
		src_delete(state);
	}
	void setRatio(float r) {
		data.src_ratio = r;
	}
	void push(const float *in, int length) {
		float out[S];
		data.data_in = in;
		data.input_frames = length;
		data.data_out = out;
		data.output_frames = S;
		src_process(state, &data);
	}
	void push(float in) {
		push(&in, 1);
	}
};


template <size_t OVERSAMPLE>
struct Decimator {
	SRC_STATE *state;
	SRC_DATA data;

	Decimator() {
		int error;
		state = src_new(SRC_SINC_FASTEST, 1, &error);
		assert(!error);

		data.data_in = NULL;
		data.data_out = NULL;
		data.input_frames = OVERSAMPLE;
		data.output_frames = 1;
		data.end_of_input = false;
		data.src_ratio = 1.0 / OVERSAMPLE;
	}
	~Decimator() {
		src_delete(state);
	}
	/** input must be length OVERSAMPLE */
	float process(float *input) {
		float output[1];
		data.data_in = input;
		data.data_out = output;
		src_process(state, &data);
		if (data.output_frames_gen > 0) {
			return output[0];
		}
		else {
			return 0.0;
		}
	}
	void reset() {
		src_reset(state);
	}
};


} // namespace rack
