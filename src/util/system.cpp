#include "util/common.hpp"

#if ARCH_WIN
#include <windows.h>
#include <shellapi.h>
#endif


namespace rack {


void openBrowser(std::string url) {
#if ARCH_LIN
	std::string command = "xdg-open " + url;
	(void)system(command.c_str());
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
