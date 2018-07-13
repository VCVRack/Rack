#include "global_pre.hpp"
#include "util/common.hpp"

#ifdef YAC_POSIX
#include <dirent.h>
#include <sys/stat.h>
#else
#include "dirent_win32/dirent.h"
#endif // YAC_POSIX

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
#endif


#ifdef USE_VST2
#define SKIP_SYSTEM_FXNS defined
#endif // USE_VST2

namespace rack {


std::vector<std::string> systemListEntries(std::string path) {
	std::vector<std::string> filenames;
	DIR *dir = opendir(path.c_str());
	if (dir) {
		struct dirent *d;
		while ((d = readdir(dir))) {
			std::string filename = d->d_name;
			if (filename == "." || filename == "..")
				continue;
			filenames.push_back(path + "/" + filename);
		}
		closedir(dir);
	}
	return filenames;
}

bool systemExists(std::string path) {
#ifndef SKIP_SYSTEM_FXNS
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) == 0);
#else
   return false;
#endif // SKIP_SYSTEM_FXNS
}

bool systemIsFile(std::string path) {
#ifdef ARCH_WIN
   DWORD dwAttrib = ::GetFileAttributes(path.c_str());
   return
      (dwAttrib != INVALID_FILE_ATTRIBUTES)        && 
      (0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) ;
#else
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISREG(statbuf.st_mode);
#endif // ARCH_WIN
}

bool systemIsDirectory(std::string path) {
#ifdef ARCH_WIN
   DWORD dwAttrib = ::GetFileAttributes(path.c_str());
   return
      (dwAttrib != INVALID_FILE_ATTRIBUTES)        && 
      (0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) ;
#else
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISDIR(statbuf.st_mode);
#endif // ARCH_WIN
}

void systemCopy(std::string srcPath, std::string destPath) {
#ifndef SKIP_SYSTEM_FXNS
	// Open files
	FILE *source = fopen(srcPath.c_str(), "rb");
	if (!source) return;
	defer({
		fclose(source);
	});
	FILE *dest = fopen(destPath.c_str(), "wb");
	if (!dest) return;
	defer({
		fclose(dest);
	});
	// Copy buffer
	const int bufferSize = (1<<15);
	char buffer[bufferSize];
	while (1) {
		size_t size = fread(buffer, 1, bufferSize, source);
		if (size == 0)
			break;
		size = fwrite(buffer, 1, size, dest);
		if (size == 0)
			break;
	}
#else
#endif // SKIP_SYSTEM_FXNS
}

void systemCreateDirectory(std::string path) {
#ifndef SKIP_SYSTEM_FXNS
#if ARCH_WIN
	CreateDirectory(path.c_str(), NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
#else
#endif // SKIP_SYSTEM_FXNS
}

void systemOpenBrowser(std::string url) {
#ifndef SKIP_SYSTEM_FXNS
#if ARCH_LIN
	std::string command = "xdg-open " + url;
	(void) system(command.c_str());
#endif
#if ARCH_MAC
	std::string command = "open " + url;
	system(command.c_str());
#endif
#if ARCH_WIN
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
#else
#endif // SKIP_SYSTEM_FXNS
}


} // namespace rack
