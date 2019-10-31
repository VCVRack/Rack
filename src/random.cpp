#include <random.hpp>
#include <math.hpp>
#include <time.h>
#include <sys/time.h>


namespace rack {
namespace random {


// xoroshiro128+
// from http://xoroshiro.di.unimi.it/xoroshiro128plus.c

thread_local uint64_t xoroshiro128plus_state[2];

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

void init() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	xoroshiro128plus_state[0] = tv.tv_sec;
	xoroshiro128plus_state[1] = tv.tv_usec;
	// Generate a few times to fix the fact that the time is not a uniform u64
	for (int i = 0; i < 10; i++) {
		xoroshiro128plus_next();
	}
}

uint32_t u32() {
	return xoroshiro128plus_next() >> 32;
}

uint64_t u64() {
	return xoroshiro128plus_next();
}

float uniform() {
	return (xoroshiro128plus_next() >> (64 - 24)) / std::pow(2.f, 24);
}

float normal() {
	// Box-Muller transform
	float radius = std::sqrt(-2.f * std::log(1.f - uniform()));
	float theta = 2.f * M_PI * uniform();
	return radius * std::sin(theta);

	// // Central Limit Theorem
	// const int n = 8;
	// float sum = 0.0;
	// for (int i = 0; i < n; i++) {
	// 	sum += uniform();
	// }
	// return (sum - n / 2.f) / std::sqrt(n / 12.f);
}


} // namespace random
} // namespace rack
