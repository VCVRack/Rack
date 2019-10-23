#include <string.hpp>
#include <locale> // for wstring_convert
#include <codecvt> // for codecvt_utf8_utf16
#include <cctype> // for tolower and toupper
#include <algorithm> // for transform
#include <libgen.h> // for dirname and basename
#include <zlib.h>


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
	size_t strLen = str.size();
	if (strLen % 4 != 0)
		throw std::runtime_error("String length not a factor of 4");

	size_t numBlocks = strLen / 4;
	size_t len = numBlocks * 3;
	if (strLen >= 4) {
		if (str[strLen - 1] == '=')
			len--;
		if (str[strLen - 2] == '=')
			len--;
	}

	std::vector<uint8_t> data(len);

	for (size_t b = 0; b < numBlocks; b++) {
		// Encode block
		uint32_t block = 0;
		size_t i;
		for (i = 0; i < 4; i++) {
			uint8_t c = str[4 * b + i];
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
				// Padding ends block encoding
				break;
			}
			else {
				// Since we assumed the string was a factor of 4 bytes, fail if an invalid character, such as whitespace, was found.
				throw std::runtime_error("String contains non-base64 character");
			}

			block |= uint32_t(d) << (6 * (3 - i));
		}

		// Decode block
		for (size_t k = 0; k < i - 1; k++) {
			data[3 * b + k] = (block >> (8 * (2 - k))) & 0xff;
		}
	}

	return data;
}


std::vector<uint8_t> compress(const uint8_t* data, size_t dataLen) {
	std::vector<uint8_t> compressed;
	uLongf outCap = ::compressBound(dataLen);
	compressed.resize(outCap);
	int err = ::compress2(compressed.data(), &outCap, data, dataLen, Z_BEST_COMPRESSION);
	if (err)
		throw std::runtime_error("Zlib error");
	compressed.resize(outCap);
	return compressed;
}


std::vector<uint8_t> compress(const std::vector<uint8_t>& data) {
	return compress(data.data(), data.size());
}


void uncompress(const uint8_t* compressed, size_t compressedLen, uint8_t* data, size_t* dataLen) {
	uLongf dataLenF = *dataLen;
	int err = ::uncompress(data, &dataLenF, compressed, compressedLen);
	(void) err;
	*dataLen = dataLenF;
}


void uncompress(const std::vector<uint8_t>& compressed, uint8_t* data, size_t* dataLen) {
	uncompress(compressed.data(), compressed.size(), data, dataLen);
}


} // namespace string
} // namespace rack
