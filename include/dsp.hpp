#pragma once

#include <assert.h>
#include <string.h>
#include <samplerate.h>


namespace rack {


/** A simple cyclic buffer.
S must be a power of 2.
push() is constant time O(1)
*/
template <typename T, size_t S>
struct RingBuffer {
	T data[S];
	size_t start = 0;
	size_t end = 0;

	size_t mask(size_t i) const {
		return i & (S - 1);
	}
	void push(T t) {
		size_t i = mask(end++);
		data[i] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	size_t size() const {
		return end - start;
	}
};


/** A cyclic buffer which maintains a valid linear array of size S by keeping a copy of the buffer in adjacent memory.
S must be a power of 2.
push() is constant time O(2) relative to RingBuffer
*/
template <typename T, size_t S>
struct DoubleRingBuffer {
	T data[S*2];
	size_t start = 0;
	size_t end = 0;

	size_t mask(size_t i) const {
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
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	size_t size() const {
		return end - start;
	}
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T *endData() {
		return &data[mask(end)];
	}
	void endIncr(size_t n) {
		size_t mend = mask(end) + n;
		if (mend > S) {
			// Copy data backward from the doubled block to the main block
			memcpy(data, &data[S], sizeof(T) * (mend - S));
			// Don't bother copying forward
		}
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[mask(start)];
	}
	void startIncr(size_t n) {
		start += n;
	}
};


/** A cyclic buffer which maintains a valid linear array of size S by sliding along a larger block of size N.
The linear array of S elements are moved back to the start of the block once it outgrows past the end.
This happens every N - S pushes, so the push() time is O(1 + S / (N - S)).
For example, a float buffer of size 64 in a block of size 1024 is nearly as efficient as RingBuffer.
*/
template <typename T, size_t S, size_t N>
struct AppleRingBuffer {
	T data[N];
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
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	size_t size() const {
		return end - start;
	}
	/** Returns a pointer to S consecutive elements for appending, requesting to append n elements.
	*/
	T *endData(size_t n) {
		// TODO
		return &data[end];
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[start];
	}
	void startIncr(size_t n) {
		// This is valid as long as n < S
		start += n;
	}
};


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
	void process(float *in, int inLength, float *out, int outLength) {
		data.data_in = in;
		data.input_frames = inLength;
		data.data_out = out;
		data.output_frames = outLength;
		src_process(state, &data);
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
