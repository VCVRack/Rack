#pragma once
#include <dsp/common.hpp>
#include <string.h>


namespace rack {
namespace dsp {


/** A simple cyclic buffer.
S must be a power of 2.
Thread-safe for single producers and consumers.
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
	void pushBuffer(const T *t, int n) {
		size_t i = mask(end);
		size_t e1 = i + n;
		size_t e2 = (e1 < S) ? e1 : S;
		std::memcpy(&data[i], t, sizeof(T) * (e2 - i));
		if (e1 > S) {
			std::memcpy(data, &t[S - i], sizeof(T) * (e1 - S));
		}
		end += n;
	}
	T shift() {
		return data[mask(start++)];
	}
	void shiftBuffer(T *t, size_t n) {
		size_t i = mask(start);
		size_t s1 = i + n;
		size_t s2 = (s1 < S) ? s1 : S;
		std::memcpy(t, &data[i], sizeof(T) * (s2 - i));
		if (s1 > S) {
			std::memcpy(&t[S - i], data, sizeof(T) * (s1 - S));
		}
		start += n;
	}
	void clear() {
		start = end;
	}
	bool empty() const {
		return start == end;
	}
	bool full() const {
		return end - start == S;
	}
	size_t size() const {
		return end - start;
	}
	size_t capacity() const {
		return S - size();
	}
};

/** A cyclic buffer which maintains a valid linear array of size S by keeping a copy of the buffer in adjacent memory.
S must be a power of 2.
Thread-safe for single producers and consumers?
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
	void clear() {
		start = end;
	}
	bool empty() const {
		return start == end;
	}
	bool full() const {
		return end - start == S;
	}
	size_t size() const {
		return end - start;
	}
	size_t capacity() const {
		return S - size();
	}
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T *endData() {
		return &data[mask(end)];
	}
	void endIncr(size_t n) {
		size_t e = mask(end);
		size_t e1 = e + n;
		size_t e2 = (e1 < S) ? e1 : S;
		// Copy data forward
		std::memcpy(&data[S + e], &data[e], sizeof(T) * (e2 - e));

		if (e1 > S) {
			// Copy data backward from the doubled block to the main block
			std::memcpy(data, &data[S], sizeof(T) * (e1 - S));
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
Not thread-safe.
*/
template <typename T, size_t S, size_t N>
struct AppleRingBuffer {
	T data[N];
	size_t start = 0;
	size_t end = 0;

	void returnBuffer() {
		// move end block to beginning
		// may overlap, but memmove handles that correctly
		size_t s = size();
		std::memmove(data, &data[start], sizeof(T) * s);
		start = 0;
		end = s;
	}
	void push(T t) {
		if (end + 1 > N) {
			returnBuffer();
		}
		data[end++] = t;
	}
	T shift() {
		return data[start++];
	}
	bool empty() const {
		return start == end;
	}
	bool full() const {
		return end - start == S;
	}
	size_t size() const {
		return end - start;
	}
	size_t capacity() const {
		return S - size();
	}
	/** Returns a pointer to S consecutive elements for appending, requesting to append n elements.
	*/
	T *endData(size_t n) {
		if (end + n > N) {
			returnBuffer();
		}
		return &data[end];
	}
	/** Actually increments the end position
	Must be called after endData(), and `n` must be at most the `n` passed to endData()
	*/
	void endIncr(size_t n) {
		end += n;
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


} // namespace dsp
} // namespace rack
