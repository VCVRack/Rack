#pragma once
#include <atomic>

#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** Lock-free queue with fixed size and no allocations.
If S is not a power of 2, performance might be reduced, and the index could overflow in a thousand years, but it should usually be fine for your purposes.

Supports only a single producer and consumer.
To my knowledge, nobody has invented a 100% correct multiple producer/consumer lock-free ring buffer for x86_64.
*/
template <typename T, size_t S>
struct RingBuffer {
	std::atomic<size_t> start{0};
	std::atomic<size_t> end{0};
	T data[S];

	/** Adds an element to the end of the buffer.
	*/
	void push(T t) {
		size_t i = end % S;
		data[i] = t;
		end++;
	}
	/** Copies an array to the end of the buffer.
	`n` must be at most S.
	*/
	void pushBuffer(const T* t, int n) {
		size_t i = end % S;
		size_t e1 = i + n;
		size_t e2 = (e1 < S) ? e1 : S;
		std::memcpy(&data[i], t, sizeof(T) * (e2 - i));
		if (e1 > S) {
			std::memcpy(data, &t[S - i], sizeof(T) * (e1 - S));
		}
		end += n;
	}
	/** Removes and returns an element from the start of the buffer.
	*/
	T shift() {
		size_t i = start % S;
		T t = data[i];
		start++;
		return t;
	}
	/** Removes and copies an array from the start of the buffer.
	`n` must be at most S.
	*/
	void shiftBuffer(T* t, size_t n) {
		size_t i = start % S;
		size_t s1 = i + n;
		size_t s2 = (s1 < S) ? s1 : S;
		std::memcpy(t, &data[i], sizeof(T) * (s2 - i));
		if (s1 > S) {
			std::memcpy(&t[S - i], data, sizeof(T) * (s1 - S));
		}
		start += n;
	}
	void clear() {
		start = end.load();
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
	size_t capacity() const {
		return S - size();
	}
};

/** A cyclic buffer which maintains a valid linear array of size S by keeping a copy of the buffer in adjacent memory.
This is not thread-safe.
*/
template <typename T, size_t S>
struct DoubleRingBuffer {
	std::atomic<size_t> start{0};
	std::atomic<size_t> end{0};
	T data[2 * S];

	void push(T t) {
		size_t i = end % S;
		data[i] = t;
		data[i + S] = t;
		end++;
	}
	T shift() {
		size_t i = start % S;
		T t = data[i];
		start++;
		return t;
	}
	void clear() {
		start = end.load();
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
	size_t capacity() const {
		return S - size();
	}
	/** Returns a pointer to S consecutive elements for appending.
	If any data is appended, you must call endIncr afterwards.
	Pointer is invalidated when any other method is called.
	*/
	T* endData() {
		size_t i = end % S;
		return &data[i];
	}
	void endIncr(size_t n) {
		size_t i = end % S;
		size_t e1 = i + n;
		size_t e2 = (e1 < S) ? e1 : S;
		// Copy data forward
		std::memcpy(&data[S + i], &data[i], sizeof(T) * (e2 - i));

		if (e1 > S) {
			// Copy data backward from the doubled block to the main block
			std::memcpy(data, &data[S], sizeof(T) * (e1 - S));
		}
		end += n;
	}
	/** Returns a pointer to S consecutive elements for consumption
	If any data is consumed, call startIncr afterwards.
	*/
	const T* startData() const {
		size_t i = start % S;
		return &data[i];
	}
	void startIncr(size_t n) {
		start += n;
	}
};

/** A cyclic buffer which maintains a valid linear array of size S by sliding along a larger block of size N.
This is not thread-safe.
The linear array of S elements are moved back to the start of the block once it outgrows past the end.
This happens every N - S pushes, so the push() time is O(1 + S / (N - S)).
For example, a float buffer of size 64 in a block of size 1024 is nearly as efficient as RingBuffer.
Not thread-safe.
*/
template <typename T, size_t S, size_t N>
struct AppleRingBuffer {
	size_t start = 0;
	size_t end = 0;
	T data[N];

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
	T* endData(size_t n) {
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
	const T* startData() const {
		return &data[start];
	}
	void startIncr(size_t n) {
		// This is valid as long as n < S
		start += n;
	}
};


} // namespace dsp
} // namespace rack
