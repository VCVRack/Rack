#include "util/common.hpp"
#include <stdarg.h>
#include <algorithm>
#include <libgen.h> // for dirname and basename


namespace rack {


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

std::string stringLowercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

std::string stringUppercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

std::string stringEllipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

bool stringStartsWith(std::string str, std::string prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

bool stringEndsWith(std::string str, std::string suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

std::string stringDirectory(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

std::string stringFilename(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

std::string stringExtension(std::string path) {
	const char *ext = strrchr(stringFilename(path).c_str(), '.');
	if (!ext)
		return "";
	return ext + 1;
}


} // namespace rack
