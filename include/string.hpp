#pragma once

#include "util/common.hpp"
#include <cstdarg>
#include <libgen.h> // for dirname and basename


namespace rack {
namespace string {


/** Converts a printf format string and optional arguments into a std::string */
inline std::string stringf(const char *format, ...) {
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

inline std::string lowercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

inline std::string uppercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

/** Truncates and adds "..." to a string, not exceeding `len` characters */
inline std::string ellipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

inline bool startsWith(std::string str, std::string prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

inline bool endsWith(std::string str, std::string suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

/** Extracts portions of a path */
inline std::string directory(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

inline std::string filename(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

inline std::string extension(std::string path) {
	const char *ext = strrchr(filename(path).c_str(), '.');
	if (!ext)
		return "";
	return ext + 1;
}

struct CaseInsensitiveCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		return lowercase(a) < lowercase(b);
	}
};


} // namespace string
} // namespace rack
