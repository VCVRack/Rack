#include "system.hpp"
#include <dirent.h>
#include <sys/stat.h>

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
#endif


namespace rack {
namespace system {


std::list<std::string> listEntries(std::string path) {
	std::list<std::string> filenames;
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

bool isFile(std::string path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISREG(statbuf.st_mode);
}

bool isDirectory(std::string path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISDIR(statbuf.st_mode);
}

void copyFile(std::string srcPath, std::string destPath) {
	// Open files
	FILE *source = fopen(srcPath.c_str(), "rb");
	if (!source)
		return;
	DEFER({
		fclose(source);
	});
	FILE *dest = fopen(destPath.c_str(), "wb");
	if (!dest)
		return;
	DEFER({
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
}

void createDirectory(std::string path) {
#if ARCH_WIN
	CreateDirectory(path.c_str(), NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
}

void openBrowser(std::string url) {
#if ARCH_LIN
	std::string command = "xdg-open " + url;
	(void) std::system(command.c_str());
#endif
#if ARCH_MAC
	std::string command = "open " + url;
	std::system(command.c_str());
#endif
#if ARCH_WIN
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}


} // namespace system
} // namespace rack
