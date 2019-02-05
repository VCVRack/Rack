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
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
	return s;
}

std::string uppercase(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
	return s;
}

std::string ellipsize(const std::string &s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

bool startsWith(const std::string &str, const std::string &prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

bool endsWith(const std::string &str, const std::string &suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

std::string directory(const std::string &path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

std::string filename(const std::string &path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

std::string basename(const std::string &path) {
	size_t pos = path.rfind('.');
	if (pos == std::string::npos)
		return path;
	return std::string(path, 0, pos);
}

std::string extension(const std::string &path) {
	size_t pos = path.rfind('.');
	if (pos == std::string::npos)
		return "";
	return std::string(path, pos);
}


} // namespace string
} // namespace rack
