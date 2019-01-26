#include "system.hpp"
#include <thread>
#include <dirent.h>
#include <sys/stat.h>

#if defined ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
#endif


namespace rack {
namespace system {


std::list<std::string> listEntries(const std::string &path) {
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

bool isFile(const std::string &path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISREG(statbuf.st_mode);
}

bool isDirectory(const std::string &path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISDIR(statbuf.st_mode);
}

void copyFile(const std::string &srcPath, const std::string &destPath) {
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

void createDirectory(const std::string &path) {
#if defined ARCH_WIN
	CreateDirectory(path.c_str(), NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
}

int getPhysicalCoreCount() {
	// TODO Return the physical cores, not logical cores.
	return std::thread::hardware_concurrency();
}

void setThreadName(const std::string &name) {
#if defined ARCH_LIN
	pthread_setname_np(pthread_self(), name.c_str());
#endif
}

void openBrowser(const std::string &url) {
#if defined ARCH_LIN
	std::string command = "xdg-open " + url;
	(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
	std::string command = "open " + url;
	std::system(command.c_str());
#endif
#if defined ARCH_WIN
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}


} // namespace system
} // namespace rack
