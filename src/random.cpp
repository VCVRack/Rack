#include <atomic>

#include <time.h>
#include <sys/time.h>

#include <random.hpp>
#include <math.hpp>


namespace rack {
namespace random {


thread_local Xoroshiro128Plus rng;
static std::atomic<uint64_t> threadCounter {0};


void init() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	rng = Xoroshiro128Plus(uint64_t(tv.tv_sec) * 1000000 + tv.tv_usec, threadCounter++);
	// Shift state a few times due to low seed entropy
	for (int i = 0; i < 4; i++) {
		rng();
	}
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


void buffer(uint8_t* out, size_t len) {
	for (size_t i = 0; i < len; i += 4) {
		uint64_t r = u64();
		out[i] = r;
		if (i + 1 < len)
			out[i + 1] = r >> 8;
		if (i + 2 < len)
			out[i + 2] = r >> 16;
		if (i + 3 < len)
			out[i + 3] = r >> 24;
	}
}


std::vector<uint8_t> vector(size_t len) {
	std::vector<uint8_t> v(len);
	buffer(v.data(), len);
	return v;
}


} // namespace random
} // namespace rack
