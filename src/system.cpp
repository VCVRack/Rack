#include <system.hpp>
#include <string.hpp>

#include <thread>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <cxxabi.h> // for __cxxabiv1::__cxa_demangle

#if defined ARCH_LIN || defined ARCH_MAC
	#include <pthread.h>
	#include <sched.h>
	#include <execinfo.h> // for backtrace and backtrace_symbols
	#include <unistd.h> // for execl
	#include <sys/utsname.h>
#endif

#if defined ARCH_MAC
	#include <mach/mach_init.h>
	#include <mach/thread_act.h>
#endif

#if defined ARCH_WIN
	#define _WIN32_WINNT _WIN32_WINNT_VISTA // for QueryThreadCycleTime
	#include <windows.h>
	#include <shellapi.h>
	#include <processthreadsapi.h>
	#include <dbghelp.h>
#endif

#define ZIP_STATIC
#include <zip.h>


namespace rack {
namespace system {


std::list<std::string> getEntries(const std::string& path) {
	std::list<std::string> filenames;
	DIR* dir = opendir(path.c_str());
	if (dir) {
		struct dirent* d;
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


std::list<std::string> getEntriesRecursive(const std::string &path, int depth) {
	std::list<std::string> entries = getEntries(path);
	if (depth > 0) {
		// Don't iterate using iterators because the list will be growing.
		size_t limit = entries.size();
		auto it = entries.begin();
		for (size_t i = 0; i < limit; i++) {
			const std::string &entry = *it++;
			if (isDirectory(entry)) {
				std::list<std::string> subEntries = getEntriesRecursive(entry, depth - 1);
				// Append subEntries to entries
				entries.splice(entries.end(), subEntries);
			}
		}
	}
	return entries;
}


bool isFile(const std::string& path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISREG(statbuf.st_mode);
}


bool isDirectory(const std::string& path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf))
		return false;
	return S_ISDIR(statbuf.st_mode);
}


void moveFile(const std::string& srcPath, const std::string& destPath) {
	std::remove(destPath.c_str());
	// Whether this overwrites existing files is implementation-defined.
	// i.e. Mingw64 fails to overwrite.
	// This is why we remove the file above.
	std::rename(srcPath.c_str(), destPath.c_str());
}


void copyFile(const std::string& srcPath, const std::string& destPath) {
	// Open source
	FILE* source = fopen(srcPath.c_str(), "rb");
	if (!source)
		return;
	DEFER({
		fclose(source);
	});
	// Open destination
	FILE* dest = fopen(destPath.c_str(), "wb");
	if (!dest)
		return;
	DEFER({
		fclose(dest);
	});
	// Copy buffer
	const int bufferSize = (1 << 15);
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


void createDirectory(const std::string& path) {
#if defined ARCH_WIN
	std::wstring pathW = string::toWstring(path);
	CreateDirectoryW(pathW.c_str(), NULL);
#else
	mkdir(path.c_str(), 0755);
#endif
}


int getLogicalCoreCount() {
	return std::thread::hardware_concurrency();
}


void setThreadName(const std::string& name) {
#if defined ARCH_LIN
	pthread_setname_np(pthread_self(), name.c_str());
#elif defined ARCH_WIN
	// Unsupported on Windows
#endif
}


void setThreadRealTime(bool realTime) {
#if defined ARCH_LIN
	int err;
	int policy;
	struct sched_param param;
	if (realTime) {
		// Round-robin scheduler policy
		policy = SCHED_RR;
		param.sched_priority = sched_get_priority_max(policy);
	}
	else {
		// Default scheduler policy
		policy = 0;
		param.sched_priority = 0;
	}
	err = pthread_setschedparam(pthread_self(), policy, &param);
	assert(!err);

	// pthread_getschedparam(pthread_self(), &policy, &param);
	// DEBUG("policy %d priority %d", policy, param.sched_priority);
#elif defined ARCH_MAC
	// Not yet implemented
#elif defined ARCH_WIN
	// Set process class first
	if (realTime) {
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	}
	else {
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	}
#endif
}


double getThreadTime() {
#if defined ARCH_LIN
	struct timespec ts;
	clockid_t cid;
	pthread_getcpuclockid(pthread_self(), &cid);
	clock_gettime(cid, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
#elif defined ARCH_MAC
	mach_port_t thread = mach_thread_self();
	mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
	thread_basic_info_data_t info;
	kern_return_t kr = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);
	if (kr != KERN_SUCCESS || (info.flags & TH_FLAGS_IDLE) != 0)
		return 0.0;
	return info.user_time.seconds + info.user_time.microseconds * 1e-6;
#elif defined ARCH_WIN
	// FILETIME creationTime;
	// FILETIME exitTime;
	// FILETIME kernelTime;
	// FILETIME userTime;
	// GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime);
	// return ((uint64_t(userTime.dwHighDateTime) << 32) + userTime.dwLowDateTime) * 1e-7;

	uint64_t cycles;
	QueryThreadCycleTime(GetCurrentThread(), &cycles);
	// HACK Assume that the RDTSC Time-Step Counter instruction is fixed at 2.5GHz. This should only be within a factor of 2 on all PCs.
	const double freq = 2.5e9;
	return (double) cycles / freq;
#endif
}


std::string getStackTrace() {
	int stackLen = 128;
	void* stack[stackLen];
	std::string s;

#if defined ARCH_LIN || defined ARCH_MAC
	stackLen = backtrace(stack, stackLen);
	char** strings = backtrace_symbols(stack, stackLen);

	// Skip the first line because it's this function.
	for (int i = 1; i < stackLen; i++) {
		s += string::f("%d: ", stackLen - i - 1);
		std::string line = strings[i];
#if 0
		// Parse line
		std::regex r(R"((.*)\((.*)\+(.*)\) (.*))");
		std::smatch match;
		if (std::regex_search(line, match, r)) {
			s += match[1].str();
			s += "(";
			std::string symbol = match[2].str();
			// Demangle symbol
			char* symbolD = __cxxabiv1::__cxa_demangle(symbol.c_str(), NULL, NULL, NULL);
			if (symbolD) {
				symbol = symbolD;
				free(symbolD);
			}
			s += symbol;
			s += "+";
			s += match[3].str();
			s += ")";
		}
#else
		s += line;
#endif
		s += "\n";
	}
	free(strings);

#elif defined ARCH_WIN
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, true);
	stackLen = CaptureStackBackTrace(0, stackLen, stack, NULL);

	SYMBOL_INFO* symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256, 1);
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


void openBrowser(const std::string& url) {
#if defined ARCH_LIN
	std::string command = "xdg-open \"" + url + "\"";
	(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
	std::string command = "open \"" + url + "\"";
	std::system(command.c_str());
#endif
#if defined ARCH_WIN
	std::wstring urlW = string::toWstring(url);
	ShellExecuteW(NULL, L"open", urlW.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
}


void openFolder(const std::string& path) {
#if defined ARCH_LIN
	std::string command = "xdg-open \"" + path + "\"";
	(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
	std::string command = "open \"" + path + "\"";
	std::system(command.c_str());
#endif
#if defined ARCH_WIN
	std::wstring pathW = string::toWstring(path);
	ShellExecuteW(NULL, L"explore", pathW.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
}


void runProcessDetached(const std::string& path) {
#if defined ARCH_WIN
	SHELLEXECUTEINFOW shExInfo;
	ZeroMemory(&shExInfo, sizeof(shExInfo));
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.lpVerb = L"runas";

	std::wstring pathW = string::toWstring(path);
	shExInfo.lpFile = pathW.c_str();
	shExInfo.nShow = SW_SHOW;

	if (ShellExecuteExW(&shExInfo)) {
		// Do nothing
	}
#else
	// Not implemented on Linux or Mac
	assert(0);
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
	// See https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_osversioninfoa for a list of Windows version numbers.
	return string::f("Windows %u.%u", info.dwMajorVersion, info.dwMinorVersion);
#endif
}


int unzipToFolder(const std::string& zipPath, const std::string& dir) {
	int err;
	// Open ZIP file
	zip_t* za = zip_open(zipPath.c_str(), 0, &err);
	if (!za) {
		WARN("Could not open ZIP file %s: error %d", zipPath.c_str(), err);
		return err;
	}
	DEFER({
		zip_close(za);
	});

	// Iterate ZIP entries
	for (int i = 0; i < zip_get_num_entries(za, 0); i++) {
		zip_stat_t zs;
		err = zip_stat_index(za, i, 0, &zs);
		if (err) {
			WARN("zip_stat_index() failed: error %d", err);
			return err;
		}

		std::string path = dir + "/" + zs.name;

		if (path[path.size() - 1] == '/') {
			// Create directory
			system::createDirectory(path);
			// HACK
			// Create and delete file to update the directory's mtime.
			std::string tmpPath = path + "/.tmp";
			FILE* tmpFile = fopen(tmpPath.c_str(), "w");
			fclose(tmpFile);
			std::remove(tmpPath.c_str());
		}
		else {
			// Open ZIP entry
			zip_file_t* zf = zip_fopen_index(za, i, 0);
			if (!zf) {
				WARN("zip_fopen_index() failed");
				return -1;
			}
			DEFER({
				zip_fclose(zf);
			});

			// Create file
			FILE* outFile = fopen(path.c_str(), "wb");
			if (!outFile) {
				WARN("Could not create file %s", path.c_str());
				return -1;
			}
			DEFER({
				fclose(outFile);
			});

			// Read buffer and copy to file
			while (true) {
				char buffer[1 << 15];
				int len = zip_fread(zf, buffer, sizeof(buffer));
				if (len <= 0)
					break;
				fwrite(buffer, 1, len, outFile);
			}
		}
	}
	return 0;
}


} // namespace system
} // namespace rack
