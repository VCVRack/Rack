#pragma once
#include <common.hpp>
#include <random>
#include <vector>


namespace rack {


/** Random number generator
*/
namespace random {


/** xoroshiro128+. Very fast, not-cryptographic random number generator.
From https://prng.di.unimi.it/
Example:

	std::random_device rd;
	random::Xoroshiro128Plus rng(rd());
	uint64_t r = rng();
	uint32_t r = rng.u32();

	std::uniform_real_distribution<float> uniform(0.f, 1.f);
	float r = uniform(rng);

	std::normal_distribution<> normal(0.0, 1.0);
	double r = normal(rng);
*/
struct Xoroshiro128Plus {
	uint64_t state[2];

	Xoroshiro128Plus(uint64_t s0 = 1, uint64_t s1 = 0) {
		seed(s0, s1);
	}
	void seed(uint64_t s0 = 1, uint64_t s1 = 0) {
		state[0] = s0;
		state[1] = s1;
		operator()();
	}

	static uint64_t rotl(const uint64_t x, int k) {
		return (x << k) | (x >> (64 - k));
	}

	uint64_t operator()() {
		const uint64_t s0 = state[0];
		uint64_t s1 = state[1];
		const uint64_t result = s0 + s1;

		s1 ^= s0;
		state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
		state[1] = rotl(s1, 36);

		return result;
	}
	constexpr uint64_t min() {
		return 0;
	}
	constexpr uint64_t max() {
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


// Easy random API

extern thread_local Xoroshiro128Plus rng;


/** Initializes the thread-local RNG state.
Must call per-thread, otherwise the RNG will always return 0.
*/
void init();
/** Returns a uniform random uint64_t from 0 to UINT64_MAX */
inline uint64_t u64() {
	return rng.u64();
}
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
inline uint32_t u32() {
	return rng.u32();
}
/** Returns a uniform random float in the interval [0.0, 1.0) */
inline float uniform() {
	return rng.f32();
}
/** Returns a normal random number with mean 0 and standard deviation 1 */
float normal();
/** Fills an array with random bytes. */
void buffer(uint8_t* out, size_t len);
/** Creates a vector of random bytes. */
std::vector<uint8_t> vector(size_t len);


} // namespace random
} // namespace rack
