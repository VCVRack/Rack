#pragma once
#include <common.hpp>
#include <random>
#include <vector>


namespace rack {
/** Random number generation */
namespace random {


/** xoroshiro128+. Very fast, not-cryptographic random number generator.
From https://prng.di.unimi.it/
Example:

	std::random_device rd;
	random::Xoroshiro128Plus rng(rd(), rd());
	uint64_t r = rng();
	uint32_t r = rng.u32();

	std::uniform_real_distribution<float> uniform(0.f, 1.f);
	float r = uniform(rng);

	std::normal_distribution<> normal(0.0, 1.0);
	double r = normal(rng);
*/
struct Xoroshiro128Plus {
	uint64_t state[2] = {};

	void seed(uint64_t s0, uint64_t s1) {
		state[0] = s0;
		state[1] = s1;
		// A bad seed will give a bad first result, so shift the state
		operator()();
	}

	bool isSeeded() {
		return state[0] || state[1];
	}

	static uint64_t rotl(uint64_t x, int k) {
		return (x << k) | (x >> (64 - k));
	}

	uint64_t operator()() {
		uint64_t s0 = state[0];
		uint64_t s1 = state[1];
		uint64_t result = s0 + s1;

		s1 ^= s0;
		state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
		state[1] = rotl(s1, 36);

		return result;
	}
	constexpr uint64_t min() const {
		return 0;
	}
	constexpr uint64_t max() const {
		return UINT64_MAX;
	}

	uint64_t u64() {
		return operator()();
	}
	uint64_t u32() {
		// Take top 32 bits which has better randomness properties.
		return u64() >> 32;
	}
	uint16_t u16() {
		return u64() >> 48;
	}
	uint8_t u8() {
		return u64() >> 56;
	}
	float f32() {
		// The multiplier is 2f7fffff in hex. This gives maximum precision of uint32_t -> float conversion and its image is [0, 1).
		return u32() * 2.32830629e-10f;
	}
	float f64() {
		return u64() * 5.421010862427522e-20;
	}
};


// Simple random API

/** Initializes the thread-local RNG state.
Must call when creating a thread, otherwise random state is undefined (might always return 0).
*/
void init();

/** Returns the thread-local generator.
*/
Xoroshiro128Plus& get();

/** Returns a uniform random uint64_t from 0 to UINT64_MAX */
inline uint64_t u64() {
	return get().u64();
}
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
inline uint32_t u32() {
	return get().u32();
}
/** Returns a uniform random float in the interval [0.0, 1.0) */
inline float uniform() {
	return get().f32();
}
/** Returns a normal random number with mean 0 and standard deviation 1 */
float normal();
/** Fills an array with random bytes. */
void buffer(uint8_t* out, size_t len);
/** Creates a vector of random bytes. */
std::vector<uint8_t> vector(size_t len);


} // namespace random
} // namespace rack
