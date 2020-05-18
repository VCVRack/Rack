#include <common.hpp>
#include <string.hpp>


namespace rack {


const std::string APP_NAME = "VCV Rack";
const std::string APP_VERSION = TOSTRING(VERSION);
#if defined ARCH_WIN
	const std::string APP_ARCH = "win";
#elif ARCH_MAC
	const std::string APP_ARCH = "mac";
#elif defined ARCH_LIN
	const std::string APP_ARCH = "lin";
#endif

const std::string ABI_VERSION = "2";

const std::string API_URL = "https://api.vcvrack.com";
const std::string API_VERSION = "2";


} // namespace rack


#if defined ARCH_WIN
#include <windows.h>

FILE* fopen_utf8(const char* filename, const char* mode) {
	return _wfopen((wchar_t*) rack::string::UTF8toUTF16(filename).c_str(), (wchar_t*) rack::string::UTF8toUTF16(mode).c_str());
}

int remove_utf8(const char* path) {
	return _wremove((wchar_t*) rack::string::UTF8toUTF16(path).c_str());
}

int rename_utf8(const char* oldname, const char* newname) {
	return _wrename((wchar_t*) rack::string::UTF8toUTF16(oldname).c_str(), (wchar_t*) rack::string::UTF8toUTF16(newname).c_str());
}

#endif
