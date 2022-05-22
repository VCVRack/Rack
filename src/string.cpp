#include <ctime>
#include <cctype> // for tolower and toupper
#include <algorithm> // for transform and equal
#include <libgen.h> // for dirname and basename
#include <stdarg.h>

#if defined ARCH_WIN
	#include <windows.h> // for MultiByteToWideChar
#endif

#include <string.hpp>


namespace rack {
namespace string {


std::string f(const char* format, ...) {
	va_list args;
	va_start(args, format);
	std::string s = fV(format, args);
	va_end(args);
	return s;
}


std::string fV(const char* format, va_list args) {
	// va_lists cannot be reused but we need it twice, so clone args.
	va_list args2;
	va_copy(args2, args);
	// Compute size of required buffer
	int size = vsnprintf(NULL, 0, format, args);
	if (size < 0)
		return "";
	// Create buffer
	std::string s;
	s.resize(size);
	vsnprintf(&s[0], size + 1, format, args2);
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
	if (str.size() < prefix.size())
		return false;
	return std::equal(prefix.begin(), prefix.end(), str.begin());
}


bool endsWith(const std::string& str, const std::string& suffix) {
	if (str.size() < suffix.size())
		return false;
	return std::equal(suffix.begin(), suffix.end(), str.end() - suffix.size());
}


std::string toBase64(const uint8_t* data, size_t dataLen) {
	static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	size_t numBlocks = (dataLen + 2) / 3;
	size_t strLen = numBlocks * 4;
	std::string str;
	str.reserve(strLen);

	for (size_t b = 0; b < numBlocks; b++) {
		// Encode block
		uint32_t block = 0;
		int i;
		for (i = 0; i < 3 && 3 * b + i < dataLen; i++) {
			block |= uint32_t(data[3 * b + i]) << (8 * (2 - i));
		}

		// Decode block
		str += alphabet[(block >> 18) & 0x3f];
		str += alphabet[(block >> 12) & 0x3f];
		str += (i > 1) ? alphabet[(block >> 6) & 0x3f] : '=';
		str += (i > 2) ? alphabet[(block >> 0) & 0x3f] : '=';
	}
	return str;
}


std::string toBase64(const std::vector<uint8_t>& data) {
	return toBase64(data.data(), data.size());
}


std::vector<uint8_t> fromBase64(const std::string& str) {
	std::vector<uint8_t> data;
	uint32_t block = 0;
	int i = 0;
	int padding = 0;

	for (char c : str) {
		uint8_t d = 0;

		if ('A' <= c && c <= 'Z') {
			d = c - 'A';
		}
		else if ('a' <= c && c <= 'z') {
			d = c - 'a' + 26;
		}
		else if ('0' <= c && c <= '9') {
			d = c - '0' + 52;
		}
		else if (c == '+') {
			d = 62;
		}
		else if (c == '/') {
			d = 63;
		}
		else if (c == '=') {
			padding++;
		}
		else {
			// Ignore whitespace and non-base64 characters
			continue;
		}

		block |= uint32_t(d) << (6 * (3 - i));
		i++;

		if (i >= 4) {
			// Decode block
			data.push_back((block >> (8 * (2 - 0))) & 0xff);
			if (padding < 2)
				data.push_back((block >> (8 * (2 - 1))) & 0xff);
			if (padding < 1)
				data.push_back((block >> (8 * (2 - 2))) & 0xff);
			// Reset block
			block = 0;
			i = 0;
			padding = 0;
		}
	}
	return data;
}


bool CaseInsensitiveCompare::operator()(const std::string& a, const std::string& b) const {
	for (size_t i = 0;; i++) {
		char ai = std::tolower(a[i]);
		char bi = std::tolower(b[i]);
		if (ai < bi)
			return true;
		if (ai > bi)
			return false;
		if (!ai || !bi)
			return false;
	}
}


std::vector<std::string> split(const std::string& s, const std::string& separator, size_t maxTokens) {
	if (separator.empty())
		throw Exception("split(): separator cannot be empty string");
	// Special case of empty string
	if (s == "")
		return {};
	if (maxTokens == 1)
		return {s};

	std::vector<std::string> v;
	size_t sepLen = separator.size();
	size_t start = 0;
	size_t end;
	while ((end = s.find(separator, start)) != std::string::npos) {
		// Add token to vector
		std::string token = s.substr(start, end - start);
		v.push_back(token);
		// Don't include delimiter
		start = end + sepLen;
		// Stop searching for tokens if we're at the token limit
		if (maxTokens == v.size() + 1)
			break;
	}

	v.push_back(s.substr(start));
	return v;
}


std::string formatTime(const char* format, double timestamp) {
	time_t t = timestamp;
	char str[1024];
	size_t s = std::strftime(str, sizeof(str), format, std::localtime(&t));
	return std::string(str, s);
}

std::string formatTimeISO(double timestamp) {
	// Windows doesn't support %F or %T, and %z gives the full timezone name instead of offset
	return formatTime("%Y-%m-%d %H:%M:%S %z", timestamp);
}


#if defined ARCH_WIN
std::string UTF16toUTF8(const std::wstring& w) {
	if (w.empty())
		return "";
	// Compute length of output buffer
	int len = WideCharToMultiByte(CP_UTF8, 0, &w[0], w.size(), NULL, 0, NULL, NULL);
	assert(len > 0);
	std::string s;
	// Allocate enough space for null character
	s.resize(len);
	len = WideCharToMultiByte(CP_UTF8, 0, &w[0], w.size(), &s[0], len, 0, 0);
	assert(len > 0);
	return s;
}


std::wstring UTF8toUTF16(const std::string& s) {
	if (s.empty())
		return L"";
	// Compute length of output buffer
	int len = MultiByteToWideChar(CP_UTF8, 0, &s[0], s.size(), NULL, 0);
	assert(len > 0);
	std::wstring w;
	// Allocate enough space for null character
	w.resize(len);
	len = MultiByteToWideChar(CP_UTF8, 0, &s[0], s.size(), &w[0], len);
	assert(len > 0);
	return w;
}
#endif


/** Parses `s` as a positive base-10 number. Returns -1 if invalid. */
static int stringToInt(const std::string& s) {
	if (s.empty())
		return -1;

	int i = 0;
	for (char c : s) {
		if (!std::isdigit((unsigned char) c))
			return -1;
		i *= 10;
		i += (c - '0');
	}
	return i;
}

/** Returns whether version part p1 is earlier than p2. */
static bool compareVersionPart(const std::string& p1, const std::string& p2) {
	int i1 = stringToInt(p1);
	int i2 = stringToInt(p2);

	if (i1 >= 0 && i2 >= 0) {
		// Compare integers.
		return i1 < i2;
	}
	else if (i1 < 0 && i2 < 0) {
		// Compare strings.
		return p1 < p2;
	}
	else {
		// Types are different. String is always less than int.
		return i1 < 0;
	}
}


Version::Version(const std::string& s) {
	parts = split(s, ".");
}

Version::operator std::string() const {
	return join(parts, ".");
}

bool Version::operator<(const Version& other) {
	return std::lexicographical_compare(parts.begin(), parts.end(), other.parts.begin(), other.parts.end(), compareVersionPart);
}



} // namespace string
} // namespace rack
