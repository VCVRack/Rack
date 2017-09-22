#pragma once

#include <string.h>
#include "math.hpp"


namespace rack {

/** A simple cyclic buffer.
S must be a power of 2.
push() is constant time O(1)
*/
template <typename T, int S>
struct RingBuffer {
	T data[S];
	int start = 0;
	int end = 0;

	int mask(int i) const {
		return i & (S - 1);
	}
	void push(T t) {
		int i = mask(end++);
		data[i] = t;
	}
	T shift() {
		return data[mask(start++)];
	}
	void clear() {
		start = end;
	}
	bool empty() const {
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	int size() const {
		return end - start;
	}
	int capacity() const {
		return S - size();
	}
};

/** A cyclic buffer which maintains a valid linear array of size S by keeping a copy of the buffer in adjacent memory.
S must be a power of 2.
push() is constant time O(2) relative to RingBuffer
*/
template <typename T, int S>
struct DoubleRingBuffer {
	T data[S*2];
	int start = 0;
	int end = 0;

	int mask(int i) const {
		return i & (S - 1);
	}
	void push(T t) {
		int i = mask(end++);
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
		return start >= end;
	}
	bool full() const {
		return end - start >= S;
	}
	int size() const {
		return end - start;
	}
	int capacity() const {
		return S - size();
	}
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T *endData() {
		return &data[mask(end)];
	}
	void endIncr(int n) {
		int e = mask(end);
		int e1 = e + n;
		int e2 = mini(e1, S);
		// Copy data forward
		memcpy(data + S + e, data + e, sizeof(T) * (e2 - e));

		if (e1 > S) {
			// Copy data backward from the doubled block to the main block
			memcpy(data, data + S, sizeof(T) * (e1 - S));
		}
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[mask(start)];
	}
	void startIncr(int n) {
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

} // namespace rack
