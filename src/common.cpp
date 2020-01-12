#include <common.hpp>


#if defined ARCH_WIN
#include <windows.h>

static wchar_t* utf8_to_w(const char* str) {
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0);
	wchar_t* strW = (wchar_t*) malloc(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strW, len);
	return strW;
}

FILE* fopen_utf8(const char* filename, const char* mode) {
	wchar_t* filenameW = utf8_to_w(filename);
	wchar_t* modeW = utf8_to_w(mode);
	FILE* file = _wfopen(filenameW, modeW);
	free(filenameW);
	free(modeW);
	return file;
}

int remove_utf8(const char* path) {
	wchar_t* pathW = utf8_to_w(path);
	int ret = _wremove(pathW);
	free(pathW);
	return ret;
}

int rename_utf8(const char* oldname, const char* newname) {
	wchar_t* oldnameW = utf8_to_w(oldname);
	wchar_t* newnameW = utf8_to_w(newname);
	int ret = _wrename(oldnameW, newnameW);
	free(oldnameW);
	free(newnameW);
	return ret;
}

#endif
