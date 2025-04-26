#include <arch.hpp>

#include <ctime>
#include <cctype> // for tolower and toupper
#include <algorithm> // for transform and equal
#include <map>
#include <libgen.h> // for dirname and basename
#include <stdarg.h>

#if defined ARCH_WIN
	#include <windows.h> // for MultiByteToWideChar
#endif

#include <string.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <asset.hpp>


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
	DEFER({va_end(args2);});
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


std::string truncate(const std::string& s, size_t maxCodepoints) {
	if (s.empty() || maxCodepoints == 0)
		return "";

	size_t pos = 0;
	for (size_t i = 0; i < maxCodepoints; i++) {
		// If remaining bytes are less than remaining codepoints, the string can't possibly be truncated.
		if (s.size() - pos <= maxCodepoints - i)
			return s;

		pos = UTF8NextCodepoint(s, pos);
		// Check if at end
		if (pos >= s.size())
			return s;
	}

	return s.substr(0, pos);
}


std::string truncatePrefix(const std::string& s, size_t maxCodepoints) {
	if (s.empty() || maxCodepoints == 0)
		return "";

	size_t pos = s.size();
	for (size_t i = 0; i < maxCodepoints; i++) {
		// If remaining bytes are less than remaining codepoints, the string can't possibly be truncated.
		if (pos <= maxCodepoints - i)
			return s;

		pos = UTF8PrevCodepoint(s, pos);
		// Check if at beginning
		if (pos == 0)
			return s;
	}

	return s.substr(pos);
}


std::string ellipsize(const std::string& s, size_t maxCodepoints) {
	if (maxCodepoints == 0)
		return "";
	std::string s2 = truncate(s, maxCodepoints);
	if (s2 == s)
		return s;
	// If string was truncated, back up a codepoint and add a Unicode ellipses character
	size_t pos = UTF8PrevCodepoint(s2, s2.size());
	return s2.substr(0, pos) + "…";
}


std::string ellipsizePrefix(const std::string& s, size_t maxCodepoints) {
	if (maxCodepoints == 0)
		return "";
	std::string s2 = truncatePrefix(s, maxCodepoints);
	if (s2 == s)
		return s;
	// If string was truncated, move forward a codepoint and prepend a Unicode ellipses character
	size_t pos = UTF8NextCodepoint(s2, 0);
	return "…" + s2.substr(pos);
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


std::string UTF32toUTF8(const std::u32string& s32) {
	std::string s8;
	// Pre-allocate maximum possible size
	s8.reserve(s32.length() * 4);

	for (char32_t c : s32) {
		// 7-bit codepoint to 1-byte sequence
		if (c <= 0x7F) {
			s8.push_back(c);
		}
		// 11-bit codepoint to 2-byte sequence
		else if (c <= 0x7FF) {
			s8.push_back(0xC0 | ((c >> 6) & 0x1F));
			s8.push_back(0x80 | (c & 0x3F));
		}
		// 16-bit codepoint to 3-byte sequence
		else if (c <= 0xFFFF) {
			s8.push_back(0xE0 | ((c >> 12) & 0x0F));
			s8.push_back(0x80 | ((c >> 6) & 0x3F));
			s8.push_back(0x80 | (c & 0x3F));
		}
		// 21-bit codepoint to 4-byte sequence
		else if (c <= 0x10FFFF) {
			s8.push_back(0xF0 | ((c >> 18) & 0x07));
			s8.push_back(0x80 | ((c >> 12) & 0x3F));
			s8.push_back(0x80 | ((c >> 6) & 0x3F));
			s8.push_back(0x80 | (c & 0x3F));
		}
		// invalid codepoint
		else {
			// Ignore character
		}
	}

	s8.shrink_to_fit();
	return s8;
}


std::u32string UTF8toUTF32(const std::string& s8) {
	std::u32string s32;
	// Pre-allocate maximum possible size
	s32.reserve(s8.size());

	for (size_t i = 0; i < s8.size();) {
		char32_t c32;
		size_t size;

		// Determine the number of bytes in the UTF-8 sequence
		if ((s8[i] & 0x80) == 0x00) {
			// 1-byte sequence
			c32 = s8[i];
			size = 1;
		}
		else if ((s8[i] & 0xE0) == 0xC0) {
			// 2-byte sequence
			if (i + 1 >= s8.size())
				break;
			c32 = (char32_t(s8[i] & 0x1F) << 6) |
			      (char32_t(s8[i + 1]) & 0x3F);
			size = 2;
			// Ignore overlong sequence
			if (c32 < 0x80)
				continue;
		}
		else if ((s8[i] & 0xF0) == 0xE0) {
			// 3-byte sequence
			if (i + 2 >= s8.size())
				break;
			c32 = (char32_t(s8[i] & 0x0F) << 12) |
			      ((char32_t(s8[i + 1]) & 0x3F) << 6) |
			      (char32_t(s8[i + 2]) & 0x3F);
			size = 3;
			// Ignore overlong sequence
			if (c32 < 0x800)
				continue;
			// Ignore surrogate pairs
			if (c32 >= 0xD800 && c32 <= 0xDFFF)
				continue;
		}
		else if ((s8[i] & 0xF8) == 0xF0) {
			// 4-byte sequence
			if (i + 3 >= s8.size())
				break;
			c32 = (char32_t(s8[i] & 0x07) << 18) |
			      ((char32_t(s8[i + 1]) & 0x3F) << 12) |
			      ((char32_t(s8[i + 2]) & 0x3F) << 6) |
			      (char32_t(s8[i + 3]) & 0x3F);
			size = 4;

			// Ignore overlong sequence
			if (c32 < 0x10000)
				continue;
			// Ignore codepoints beyond Unicode maximum
			if (c32 > 0x10FFFF)
				continue;
		}
		else {
			// Ignore invalid first byte
			continue;
		}

		s32.push_back(c32);
		i += size;
	}

	s32.shrink_to_fit();
	return s32;
}


static size_t UTF8CodepointSize(char c) {
	if (!c) return 0;
	// First byte signals size
	// 0b0xxxxxxx
	if ((c & 0x80) == 0x00) return 1;
	// 0b110xxxxx
	if ((c & 0xe0) == 0xc0) return 2;
	// 0b1110xxxx
	if ((c & 0xf0) == 0xe0) return 3;
	// 0b11110xxx
	if ((c & 0xf8) == 0xf0) return 4;
	// Invalid first UTF-8 byte
	return 0;
}


size_t UTF8NextCodepoint(const std::string& s8, size_t pos) {
	// Check out of bounds
	if (pos >= s8.size())
		return s8.size();
	size_t size = UTF8CodepointSize(s8[pos]);
	// Check for continuation byte 0b10xxxxxx
	// if ((s8[1] & 0xc0) != 0x80) return 0;
	// if ((s8[2] & 0xc0) != 0x80) return 0;
	// if ((s8[3] & 0xc0) != 0x80) return 0;
	return std::min(pos + size, s8.size());
}


/** Finds the byte index of the front of a codepoint by reversing until a non-continuation byte is found. */
static size_t UTF8StartCodepoint(const std::string& s8, size_t pos) {
	// Check out of bounds
	if (pos >= s8.size())
		return s8.size();
	while (pos > 0) {
		// Check for continuation byte 0b10xxxxxx
		if ((s8[pos] & 0xc0) != 0x80)
			break;
		pos--;
	}
	return pos;
}


size_t UTF8PrevCodepoint(const std::string& s8, size_t pos) {
	if (pos == 0)
		return 0;
	return UTF8StartCodepoint(s8, pos - 1);
}


size_t UTF8Length(const std::string& s8) {
	return UTF8CodepointIndex(s8, s8.size());
}


size_t UTF8CodepointIndex(const std::string& s8, size_t endPos) {
	size_t pos = 0;
	size_t index = 0;
	endPos = std::min(endPos, s8.size());
	while (pos < endPos) {
		size_t newPos = UTF8NextCodepoint(s8, pos);
		// Check if codepoint is invalid
		if (pos == newPos)
			return index;
		pos = newPos;
		index++;
	}
	return index;
}


size_t UTF8CodepointPos(const std::string& s8, size_t endIndex) {
	size_t pos = 0;
	size_t index = 0;
	while (index < endIndex && pos < s8.size()) {
		size_t newPos = UTF8NextCodepoint(s8, pos);
		// Check if codepoint is invalid
		if (pos == newPos)
			return pos;
		pos = newPos;
		index++;
	}
	return pos;
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


/** {language: {id: string}} */
static std::map<std::string, std::map<std::string, std::string>> translations;


static void loadTranslations() {
	INFO("Loading translations");
	translations.clear();
	std::string translationsDir = asset::system("translations");

	for (std::string filename : system::getEntries(translationsDir)) {
		if (system::getExtension(filename) != ".json")
			continue;
		std::string language = system::getStem(filename);
		std::string path = system::join(translationsDir, filename);

		INFO("Loading translation %s from %s", language.c_str(), path.c_str());
		FILE* file = std::fopen(path.c_str(), "r");
		if (!file) {
			WARN("Cannot open translation file %s", path.c_str());
			continue;
		}
		DEFER({std::fclose(file);});

		json_error_t error;
		json_t* rootJ = json_loadf(file, 0, &error);
		if (!rootJ) {
			WARN("Translation file %s has invalid JSON at %d:%d %s", path.c_str(), error.line, error.column, error.text);
			continue;
		}
		DEFER({json_decref(rootJ);});

		auto& translation = translations[language];

		// Load JSON
		const char* id;
		json_t* strJ;
		json_object_foreach(rootJ, id, strJ) {
			if (json_is_string(strJ)) {
				translation[id] = std::string(json_string_value(strJ), json_string_length(strJ));
			}
		}
	}
}


std::string translate(const std::string& id) {
	std::string s = translate(id, settings::language);
	if (!s.empty())
		return s;

	// English fallback
	if (settings::language != "en") {
		return translate(id, "en");
	}
	return "";
}


std::string translate(const std::string& id, const std::string& language) {
	const auto it = translations.find(language);
	if (it == translations.end())
		return "";

	const auto& translation = it->second;
	const auto it2 = translation.find(id);
	if (it2 == translation.end()) {
		WARN("Translation %s not found for %s", id.c_str(), language.c_str());
		return "";
	}
	return it2->second;
}


std::vector<std::string> getLanguages() {
	std::vector<std::string> languages;
	for (const auto& pair : translations) {
		languages.push_back(pair.first);
	}
	// Sort by language name, by UTF-8 bytes
	std::sort(languages.begin(), languages.end(), [](const std::string& a, const std::string& b) {
		return translate("language", a) < translate("language", b);
	});
	return languages;
}


void init() {
	loadTranslations();
}


} // namespace string
} // namespace rack
