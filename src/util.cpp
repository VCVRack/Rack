#include "util.hpp"
#include <stdarg.h>
#include <string.h>
#include <random>
#include <algorithm>
#include <libgen.h> // for dirname and basename
#include <sys/time.h>

#if ARCH_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace rack {


////////////////////
// RNG
////////////////////

// xoroshiro128+
// from http://xoroshiro.di.unimi.it/xoroshiro128plus.c

static uint64_t xoroshiro128plus_state[2];

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

void randomSeedTime() {
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

float randomf() {
	// 24 bits of granularity is the best that can be done with floats while ensuring that the return value lies in [0.0, 1.0).
	return (xoroshiro128plus_next() >> (64 - 24)) / powf(2, 24);
}

float randomNormal() {
	// Box-Muller transform
	float radius = sqrtf(-2.f * logf(1.f - randomf()));
	float theta = 2.f * M_PI * randomf();
	return radius * sinf(theta);

	// // Central Limit Theorem
	// const int n = 8;
	// float sum = 0.0;
	// for (int i = 0; i < n; i++) {
	// 	sum += randomf();
	// }
	// return (sum - n / 2.f) / sqrtf(n / 12.f);
}

////////////////////
// String functions
////////////////////

std::string stringf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	// Compute size of required buffer
	int size = vsnprintf(NULL, 0, format, args);
	va_end(args);
	if (size < 0)
		return "";
	// Create buffer
	std::string s;
	s.resize(size);
	va_start(args, format);
	vsnprintf(&s[0], size + 1, format, args);
	va_end(args);
	return s;
}

std::string lowercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

std::string uppercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

std::string ellipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

bool startsWith(std::string str, std::string prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

std::string extractDirectory(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

std::string extractFilename(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

std::string extractExtension(std::string path) {
	const char *ext = strrchr(path.c_str(), '.');
	if (!ext)
		return "";
	return ext + 1;
}

////////////////////
// Operating system functions
////////////////////

void openBrowser(std::string url) {
#if ARCH_LIN
	std::string command = "xdg-open " + url;
	(void)system(command.c_str());
#endif
#if ARCH_MAC
	std::string command = "open " + url;
	system(command.c_str());
#endif
#if ARCH_WIN
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}

////////////////////
// logger
////////////////////

FILE *gLogFile = stderr;

void debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[debug] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[info] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[warning] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void fatal(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[fatal] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}


} // namespace rack
