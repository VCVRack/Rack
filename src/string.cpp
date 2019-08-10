#include <string.hpp>
#include <locale> // for wstring_convert
#include <codecvt> // for codecvt_utf8_utf16
#include <cctype> // for tolower and toupper
#include <algorithm> // for transform
#include <libgen.h> // for dirname and basename


namespace rack {
namespace string {


std::string fromWstring(const std::wstring& s) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(s);
}


std::wstring toWstring(const std::string& s) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(s);
}


std::string f(const char* format, ...) {
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


std::string lowercase(const std::string& s) {
	std::string r = s;
	std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return r;
}


std::string uppercase(const std::string& s) {
	std::string r = s;
	std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) {
		return std::toupper(c);
	});
	return r;
}


std::string trim(const std::string& s) {
	const std::string whitespace = " \n\r\t";
	size_t first = s.find_first_not_of(whitespace);
	if (first == std::string::npos)
		return "";
	size_t last = s.find_last_not_of(whitespace);
	if (last == std::string::npos)
		return "";
	return s.substr(first, last - first + 1);
}


std::string ellipsize(const std::string& s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}


std::string ellipsizePrefix(const std::string& s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return "..." + s.substr(s.size() - (len - 3));
}


bool startsWith(const std::string& str, const std::string& prefix) {
	return str.substr(0, prefix.size()) == prefix;
}


bool endsWith(const std::string& str, const std::string& suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}


std::string directory(const std::string& path) {
	char* pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}


std::string filename(const std::string& path) {
	char* pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}


std::string filenameBase(const std::string& filename) {
	size_t pos = filename.rfind('.');
	if (pos == std::string::npos)
		return filename;
	return std::string(filename, 0, pos);
}


std::string filenameExtension(const std::string& filename) {
	size_t pos = filename.rfind('.');
	if (pos == std::string::npos)
		return "";
	return std::string(filename, pos + 1);
}


std::string absolutePath(const std::string& path) {
#if defined ARCH_LIN || defined ARCH_MAC
	char buf[PATH_MAX];
	char* absPathC = realpath(path.c_str(), buf);
	if (absPathC)
		return absPathC;
#elif defined ARCH_WIN
	std::wstring pathW = toWstring(path);
	wchar_t buf[PATH_MAX];
	wchar_t* absPathC = _wfullpath(buf, pathW.c_str(), PATH_MAX);
	if (absPathC)
		return fromWstring(absPathC);
#endif
	return "";
}


float fuzzyScore(const std::string& s, const std::string& query) {
	size_t pos = s.find(query);
	if (pos == std::string::npos)
		return 0.f;

	return (float)(query.size() + 1) / (s.size() + 1);
}


} // namespace string
} // namespace rack
