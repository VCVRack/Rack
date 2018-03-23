#include "util/common.hpp"

#include <dirent.h>

#if ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
#endif


namespace rack {


std::vector<std::string> systemListDirectory(std::string path) {
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
