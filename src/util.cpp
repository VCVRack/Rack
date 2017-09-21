#include "util.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <random>
#include <libgen.h> // for dirname and basename

#if ARCH_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace rack {

// TODO
// Convert this to xoroshiro128+ or something, and write custom normal dist implementation

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
	va_list args;
	va_start(args, format);
	int size = vsnprintf(NULL, 0, format, args);
	va_end(args);
	if (size < 0)
		return "";
	std::string s;
	s.resize(size);
	va_start(args, format);
	vsnprintf(&s[0], size+1, format, args);
	va_end(args);
	return s;
}

std::string ellipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
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


} // namespace rack
