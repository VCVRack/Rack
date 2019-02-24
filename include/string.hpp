#pragma once
#include "common.hpp"


namespace rack {


/** Supplemental `std::string` functions
*/
namespace string {


/** Converts a `printf()` format string and optional arguments into a std::string
Remember that "%s" must reference a `char *`, so use `.c_str()` for `std::string`s.
*/
std::string f(const char *format, ...);
/** Replaces all characters to lowercase letters */
std::string lowercase(const std::string &s);
/** Replaces all characters to uppercase letters */
std::string uppercase(const std::string &s);
std::string trim(const std::string &s);
/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(const std::string &s, size_t len);
bool startsWith(const std::string &str, const std::string &prefix);
bool endsWith(const std::string &str, const std::string &suffix);
/** Extracts portions of a path */
std::string directory(const std::string &path);
std::string filename(const std::string &path);
/** Extracts the portion of a path without the extension */
std::string basename(const std::string &path);
/** Extracts the extension of a path */
std::string extension(const std::string &path);
/** Scores how well a query matches a string.
A score of 0 means no match.
The score is arbitrary and is only meaningful for sorting.
*/
float fuzzyScore(const std::string &s, const std::string &query);

struct CaseInsensitiveCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		return lowercase(a) < lowercase(b);
	}
};


} // namespace string
} // namespace rack
