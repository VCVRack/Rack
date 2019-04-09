#include "system.hpp"
#include "string.hpp"

#include <thread>
#include <dirent.h>
#include <sys/stat.h>

#if defined ARCH_LIN || defined ARCH_MAC
	#include <pthread.h>
	#include <sched.h>
	#include <execinfo.h> // for backtrace and backtrace_symbols
	#include <unistd.h> // for execl
	#include <sys/utsname.h>
#endif

#if defined ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
	#include <processthreadsapi.h>
	#include <dbghelp.h>
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
	filenames.sort();
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
	wchar_t pathW[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pathW, LENGTHOF(pathW));
	CreateDirectoryW(pathW, NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
}

int getLogicalCoreCount() {
	// TODO Return the physical cores, not logical cores.
	return std::thread::hardware_concurrency();
}

void setThreadName(const std::string &name) {
#if defined ARCH_LIN
	pthread_setname_np(pthread_self(), name.c_str());
#elif defined ARCH_WIN
	// Unsupported on Windows
#endif
}

void setThreadRealTime() {
#if defined ARCH_LIN || defined ARCH_MAC
	// Round-robin scheduler policy
	int policy = SCHED_RR;
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(policy);
	pthread_setschedparam(pthread_self(), policy, &param);
#elif defined ARCH_WIN
	// Set process class first
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
}

std::string getStackTrace() {
	int stackLen = 128;
	void *stack[stackLen];
	std::string s;

#if defined ARCH_LIN || defined ARCH_MAC
	stackLen = backtrace(stack, stackLen);
	char **strings = backtrace_symbols(stack, stackLen);

	for (int i = 1; i < stackLen; i++) {
		s += string::f("%d: %s\n", stackLen - i - 1, strings[i]);
	}
	free(strings);
#elif defined ARCH_WIN
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, true);
	stackLen = CaptureStackBackTrace(0, stackLen, stack, NULL);

	SYMBOL_INFO *symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256, 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (int i = 1; i < stackLen; i++) {
		SymFromAddr(process, (DWORD64) stack[i], 0, symbol);
		s += string::f("%d: %s 0x%0x\n", stackLen - i - 1, symbol->Name, symbol->Address);
	}
	free(symbol);
#endif

	return s;
}

void openBrowser(const std::string &url) {
#if defined ARCH_LIN
	std::string command = "xdg-open \"" + url + "\"";
	(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
	std::string command = "open \"" + url + "\"";
	std::system(command.c_str());
#endif
#if defined ARCH_WIN
	wchar_t urlW[1024];
	MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, urlW, LENGTHOF(urlW));
	ShellExecuteW(NULL, L"open", urlW, NULL, NULL, SW_SHOWNORMAL);
#endif
}

void openFolder(const std::string &path) {
#if defined ARCH_LIN
	std::string command = "xdg-open \"" + path + "\"";
	(void) std::system(command.c_str());
#endif
#if defined ARCH_WIN
	wchar_t pathW[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pathW, LENGTHOF(pathW));
	ShellExecuteW(NULL, L"explorer", pathW, NULL, NULL, SW_SHOWNORMAL);
#endif
}


void runProcessAsync(const std::string &path) {
#if defined ARCH_WIN
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION processInfo;

	std::memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	std::memset(&processInfo, 0, sizeof(processInfo));

	wchar_t pathW[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pathW, LENGTHOF(pathW));
	CreateProcessW(pathW, NULL,
		NULL, NULL, false, 0, NULL, NULL,
		&startupInfo, &processInfo);
#endif
}


std::string getOperatingSystemInfo() {
#if defined ARCH_LIN || defined ARCH_MAC
	struct utsname u;
	uname(&u);
	return string::f("%s %s %s %s", u.sysname, u.release, u.version, u.machine);
#elif defined ARCH_WIN
	OSVERSIONINFOW info;
	ZeroMemory(&info, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionExW(&info);
	return string::f("Windows %u.%u", info.dwMajorVersion, info.dwMinorVersion);
#endif
}


} // namespace system
} // namespace rack
