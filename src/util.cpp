#include "util.hpp"
#include <stdio.h>
#include <stdarg.h>


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
	char *buf = new char[size];
	vsnprintf(buf, size, format, ap);
	va_end(ap);
	std::string s = buf;
	delete[] buf;
	return s;
}


} // namespace rack
