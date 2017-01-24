#include "util.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <random>


namespace rack {

// TODO
// Convert this to xoroshiro128+ and custom normal dist implementation

static std::random_device rd;
static std::mt19937 rng(rd());
static std::uniform_real_distribution<float> uniformDist;
static std::normal_distribution<float> normalDist;

uint32_t randomu32() {
	return rng();
}

float randomf() {
	return uniformDist(rng);
}

float randomNormal(){
	return normalDist(rng);
}


std::string stringf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	size_t size = vsnprintf(NULL, 0, format, ap);
	if (size < 0)
		return "";
	size++;
	std::string s;
	s.resize(size);
	vsnprintf(&s[0], size, format, ap);
	va_end(ap);
	return s;
}

std::string ellipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}


} // namespace rack
