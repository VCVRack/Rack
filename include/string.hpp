#pragma once
#include <common.hpp>


namespace rack {


/** Supplemental `std::string` functions
*/
namespace string {


/** Converts a UTF-16/32 string (depending on the size of wchar_t) to a UTF-8 string. */
std::string fromWstring(const std::wstring& s);
std::wstring toWstring(const std::string& s);
/** Converts a `printf()` format string and optional arguments into a std::string.
Remember that "%s" must reference a `char *`, so use `.c_str()` for `std::string`s, otherwise you might get binary garbage.
*/
std::string f(const char* format, ...);
/** Replaces all characters to lowercase letters */
std::string lowercase(const std::string& s);
/** Replaces all characters to uppercase letters */
std::string uppercase(const std::string& s);
/** Removes whitespace from beginning and end of string. */
std::string trim(const std::string& s);
/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(const std::string& s, size_t len);
std::string ellipsizePrefix(const std::string& s, size_t len);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);
/** Extracts the directory of the path.
Example: directory("dir/file.txt") // "dir"
Calls POSIX dirname().
*/
std::string directory(const std::string& path);
/** Extracts the filename of the path.
Example: directory("dir/file.txt") // "file.txt"
Calls POSIX basename().
*/
std::string filename(const std::string& path);
/** Extracts the portion of a filename without the extension.
Example: filenameBase("file.txt") // "file"
Note: Only works on filenames. Call filename(path) to get the filename of the path.
*/
std::string filenameBase(const std::string& filename);
/** Extracts the extension of a filename.
Example: filenameExtension("file.txt") // "txt"
Note: Only works on filenames. Call filename(path) to get the filename of the path.
*/
std::string filenameExtension(const std::string& filename);
/** Returns the canonicalized absolute path pointed to by `path`, following symlinks.
Returns "" if the symbol is not found.
*/
std::string absolutePath(const std::string& path);
/** Scores how well a query matches a string.
A score of 0 means no match.
The score is arbitrary and is only meaningful for sorting.
*/
float fuzzyScore(const std::string& s, const std::string& query);
/** Converts a byte array to a Base64-encoded string.
https://en.wikipedia.org/wiki/Base64
*/
std::string toBase64(const uint8_t* data, size_t len);
/** Converts a Base64-encoded string to a byte array.
`outLen` is set to the length of the byte array.
If non-NULL, caller must delete[] the result.
*/
uint8_t* fromBase64(const std::string& str, size_t* outLen);


struct CaseInsensitiveCompare {
	bool operator()(const std::string& a, const std::string& b) const {
		return lowercase(a) < lowercase(b);
	}
};


} // namespace string
} // namespace rack
