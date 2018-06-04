#include "util/common.hpp"

#include <dirent.h>
#include <sys/stat.h>

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
#endif


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
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) == 0);
}

bool systemIsFile(std::string path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISREG(statbuf.st_mode);
}

bool systemIsDirectory(std::string path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISDIR(statbuf.st_mode);
}

void systemCopy(std::string srcPath, std::string destPath) {
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
}

void systemCreateDirectory(std::string path) {
#if ARCH_WIN
	CreateDirectory(path.c_str(), NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
}

void systemOpenBrowser(std::string url) {
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
}


} // namespace rack
