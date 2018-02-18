#pragma once

#include <string.h>
#include "util/common.hpp"


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
		int e2 = min(e1, S);
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
template <typename T, int S, int N>
struct AppleRingBuffer {
	T data[N];
	int start = 0;
	int end = 0;

	void returnBuffer() {
		// move end block to beginning
		// may overlap, but that's okay
		int s = size();
		memmove(data, &data[start], sizeof(T) * s);
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
	/** Returns a pointer to S consecutive elements for appending, requesting to append n elements.
	*/
	T *endData(int n) {
		if (end + n > N) {
			returnBuffer();
		}
		return &data[end];
	}
	/** Actually increments the end position
	Must be called after endData(), and `n` must be at most the `n` passed to endData()
	*/
	void endIncr(int n) {
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T *startData() const {
		return &data[start];
	}
	void startIncr(int n) {
		// This is valid as long as n < S
		start += n;
	}
};

} // namespace rack
