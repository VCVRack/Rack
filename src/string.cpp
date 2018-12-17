#include "string.hpp"
#include <algorithm> // for transform
#include <libgen.h> // for dirname and basename


namespace rack {
namespace string {


std::string f(const char *format, ...) {
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

/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(std::string s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

bool startsWith(std::string str, std::string prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string str, std::string suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

/** Extracts portions of a path */
std::string directory(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

std::string filename(std::string path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

std::string basename(std::string path) {
	size_t pos = path.rfind('.');
	return std::string(path, 0, pos);
}

std::string extension(std::string path) {
	size_t pos = path.rfind('.');
	return std::string(path, pos);
}


} // namespace string
} // namespace rack
