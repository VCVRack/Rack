#pragma once
#include "common.hpp"


namespace rack {
namespace string {


/** Converts a printf format string and optional arguments into a std::string */
std::string f(const char *format, ...);
/** Replaces all characters to lowercase letters */
std::string lowercase(std::string s);
/** Replaces all characters to uppercase letters */
std::string uppercase(std::string s);
/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(std::string s, size_t len);
bool startsWith(std::string str, std::string prefix);
bool endsWith(std::string str, std::string suffix);
/** Extracts portions of a path */
std::string directory(std::string path);
std::string filename(std::string path);
/** Extracts the portion of a path without the extension */
std::string basename(std::string path);
/** Extracts the extension of a path */
std::string extension(std::string path);

struct CaseInsensitiveCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		return lowercase(a) < lowercase(b);
	}
};


} // namespace string
} // namespace rack
