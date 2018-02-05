#include "util/common.hpp"
#include <time.h>
#include <sys/time.h>


namespace rack {


// xoroshiro128+
// from http://xoroshiro.di.unimi.it/xoroshiro128plus.c

static uint64_t xoroshiro128plus_state[2] = {};

static uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t xoroshiro128plus_next(void) {
	const uint64_t s0 = xoroshiro128plus_state[0];
	uint64_t s1 = xoroshiro128plus_state[1];
	const uint64_t result = s0 + s1;

	s1 ^= s0;
	xoroshiro128plus_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
	xoroshiro128plus_state[1] = rotl(s1, 36); // c

	return result;
}

void randomInit() {
	// Only allow the seed to be initialized once during the lifetime of the program.
	assert(xoroshiro128plus_state[0] == 0 && xoroshiro128plus_state[1] == 0);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	xoroshiro128plus_state[0] = tv.tv_sec;
	xoroshiro128plus_state[1] = tv.tv_usec;
	// Generate a few times to fix the fact that the time is not a uniform u64
	for (int i = 0; i < 10; i++) {
		xoroshiro128plus_next();
	}
}

uint32_t randomu32() {
	return xoroshiro128plus_next() >> 32;
}

uint64_t randomu64() {
	return xoroshiro128plus_next();
}

float randomUniform() {
	// 24 bits of granularity is the best that can be done with floats while ensuring that the return value lies in [0.0, 1.0).
	return (xoroshiro128plus_next() >> (64 - 24)) / powf(2, 24);
}

float randomNormal() {
	// Box-Muller transform
	float radius = sqrtf(-2.f * logf(1.f - randomUniform()));
	float theta = 2.f * M_PI * randomUniform();
	return radius * sinf(theta);

	// // Central Limit Theorem
	// const int n = 8;
	// float sum = 0.0;
	// for (int i = 0; i < n; i++) {
	// 	sum += randomUniform();
	// }
	// return (sum - n / 2.f) / sqrtf(n / 12.f);
}


} // namespace rack
