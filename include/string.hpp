#pragma once
#include <stdarg.h>
#include <vector>

#include <common.hpp>


namespace rack {
/** Supplemental `std::string` functions */
namespace string {


/** Converts a `printf()` format string and optional arguments into a std::string.
Remember that "%s" must reference a `char *`, so use `.c_str()` for `std::string`s, otherwise you will get binary garbage.
*/
__attribute__((format(printf, 1, 2)))
std::string f(const char* format, ...);
std::string fV(const char* format, va_list args);
/** Replaces all characters to lowercase letters */
std::string lowercase(const std::string& s);
/** Replaces all characters to uppercase letters */
std::string uppercase(const std::string& s);
/** Removes whitespace from beginning and end of string. */
std::string trim(const std::string& s);
/** Truncates and adds "..." to the end of a string, not exceeding `len` characters. */
std::string ellipsize(const std::string& s, size_t len);
/** Truncates and adds "..." to the beginning of a string, not exceeding `len` characters. */
std::string ellipsizePrefix(const std::string& s, size_t len);
/** Returns whether a string starts with the given substring. */
bool startsWith(const std::string& str, const std::string& prefix);
/** Returns whether a string ends with the given substring. */
bool endsWith(const std::string& str, const std::string& suffix);

/** Converts a byte array to a Base64-encoded string.
https://en.wikipedia.org/wiki/Base64
*/
std::string toBase64(const uint8_t* data, size_t dataLen);
std::string toBase64(const std::vector<uint8_t>& data);
/** Converts a Base64-encoded string to a byte array.
Throws std::runtime_error if string is invalid.
*/
std::vector<uint8_t> fromBase64(const std::string& str);

struct CaseInsensitiveCompare {
	/** Returns whether `a < b` using case-insensitive lexical comparison. */
	bool operator()(const std::string& a, const std::string& b) const;
};

/** Joins an container (vector, list, etc) of std::strings with an optional separator string.
*/
template <typename TContainer>
std::string join(const TContainer& container, std::string seperator = "") {
	std::string s;
	bool first = true;
	for (const auto& c : container) {
		if (!first)
			s += seperator;
		first = false;
		s += c;
	}
	return s;
}

/** Splits a string into a vector of tokens.
If `maxTokens > 0`, limits the number of tokens.
Tokens do not include the separator string.
Examples:
	split("a+b+c", "+") // {"a", "b", "c"}
	split("abc", "+") // {"abc"}
	split("a++c", "+") // {"a", "", "c"}
	split("", "+") // {}
	split("abc", "") // throws rack::Exception
*/
std::vector<std::string> split(const std::string& s, const std::string& seperator, size_t maxTokens = 0);

/** Formats a UNIX timestamp with a strftime() string. */
std::string formatTime(const char* format, double timestamp);
std::string formatTimeISO(double timestamp);


#if defined ARCH_WIN
/** Performs a Unicode string conversion from UTF-16 to UTF-8.
These are only defined on Windows because the implementation uses Windows' API, and conversion is not needed on other OS's (since everything on Mac and Linux is UTF-8).

std::string and char* variables are considered UTF-8, anywhere in the program.
See https://utf8everywhere.org/ for more information about VCV Rack's philosophy on string encoding, especially section 10 for rules VCV follows for handling text on Windows.
*/
std::string UTF16toUTF8(const std::wstring& w);
std::wstring UTF8toUTF16(const std::string& s);
#endif


/** Structured version string, for comparison.

Strings are split into parts by "." and compared lexicographically.
Parts are compared as the following.

If both parts can be parsed as integers (such as "10" and "2"), their integer value is compared (so "10" is greater).
If one part cannot be parsed as an integer (such as "2" and "beta"), the integer part is greater ("2").
If neither part can be parsed as an integer (such as "alpha" and "beta"), the parts are compared as strings in typical lexicographical order.

For example, the following versions are sorted earliest to latest.

	1.a.0
	1.b.0
	1.0.0
	1.0.1
	1.2.0
	1.10.0
	2.0.0
*/
struct Version {
	std::vector<std::string> parts;

	Version() {}
	Version(const std::string& s);
	Version(const char* s) : Version(std::string(s)) {}
	operator std::string() const;
	/** Returns whether this version is earlier than `other`. */
	bool operator<(const Version& other);
};


} // namespace string
} // namespace rack
