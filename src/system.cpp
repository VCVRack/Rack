#include <thread>
#include <regex>
#include <chrono>
#include <experimental/filesystem>

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
	#include <windows.h>
	#include <shellapi.h>
	#include <processthreadsapi.h>
	#include <dbghelp.h>
#endif

#include <archive.h>
#include <archive_entry.h>

#include <system.hpp>
#include <string.hpp>


/*
In C++17, this will be `std::filesystem`
Important: When using `fs::path`, always convert strings to UTF-8 using

	fs::path p = fs::u8path(s);

In fact, it's best to work only with strings to avoid forgetting to decode a string as UTF-8.
The need to do this is a fatal flaw of `fs::path`, but at least `std::filesystem` has some helpful operations.
*/
namespace fs = std::experimental::filesystem;


namespace rack {
namespace system {


std::list<std::string> getEntries(const std::string& dirPath, int depth) {
	try {
		std::list<std::string> entries;
		for (auto& entry : fs::directory_iterator(fs::u8path(dirPath))) {
			std::string subEntry = entry.path().u8string();
			entries.push_back(subEntry);
			// Recurse if depth > 0 (limited recursion) or depth < 0 (infinite recursion).
			if (depth != 0) {
				if (fs::is_directory(entry.path())) {
					std::list<std::string> subEntries = getEntries(subEntry, depth - 1);
					entries.splice(entries.end(), subEntries);
				}
			}
		}
		return entries;
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool doesExist(const std::string& path) {
	try {
		return fs::exists(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool isFile(const std::string& path) {
	try {
		return fs::is_regular_file(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool isDirectory(const std::string& path) {
	try {
		return fs::is_directory(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


uint64_t getFileSize(const std::string& path) {
	try {
		return fs::file_size(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


void rename(const std::string& srcPath, const std::string& destPath) {
	try {
		fs::rename(fs::u8path(srcPath), fs::u8path(destPath));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


void copy(const std::string& srcPath, const std::string& destPath) {
	try {
		fs::copy(fs::u8path(srcPath), fs::u8path(destPath), fs::copy_options::recursive);
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool createDirectory(const std::string& path) {
	try {
		return fs::create_directory(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool createDirectories(const std::string& path) {
	try {
		return fs::create_directories(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


bool remove(const std::string& path) {
	try {
		return fs::remove(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


int removeRecursively(const std::string& path) {
	try {
		return fs::remove_all(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getWorkingDirectory() {
	try {
		return fs::current_path().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


void setWorkingDirectory(const std::string& path) {
	try {
		fs::current_path(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getTempDir() {
	try {
		return fs::temp_directory_path().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getAbsolute(const std::string& path) {
	try {
		return fs::absolute(fs::u8path(path)).u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getCanonical(const std::string& path) {
	try {
		return fs::canonical(fs::u8path(path)).u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getDirectory(const std::string& path) {
	try {
		return fs::u8path(path).parent_path().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getFilename(const std::string& path) {
	try {
		return fs::u8path(path).filename().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getStem(const std::string& path) {
	try {
		return fs::u8path(path).stem().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


std::string getExtension(const std::string& path) {
	try {
		return fs::u8path(path).extension().u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
}


/** Returns `p` in relative path form, relative to `base`
Limitation: `p` must be a descendant of `base`. Doesn't support adding `../` to the return path.
*/
static std::string getRelativePath(std::string path, std::string base) {
	try {
		path = fs::absolute(fs::u8path(path)).generic_u8string();
		base = fs::absolute(fs::u8path(base)).generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		throw Exception(e.what());
	}
	if (path.size() < base.size())
		throw Exception("getRelativePath() error: path is shorter than base");
	if (!std::equal(base.begin(), base.end(), path.begin()))
		throw Exception("getRelativePath() error: path does not begin with base");

	// If path == base, this correctly returns "."
	return "." + std::string(path.begin() + base.size(), path.end());
}


void archiveFolder(const std::string& archivePath, const std::string& folderPath) {
	// Based on minitar.c create() in libarchive examples
	int r;

	// Open archive for writing
	struct archive* a = archive_write_new();
	DEFER({archive_write_free(a);});
	archive_write_set_format_ustar(a);
	archive_write_add_filter_zstd(a);
#if defined ARCH_WIN
	r = archive_write_open_filename_w(a, string::U8toU16(archivePath).c_str());
#else
	r = archive_write_open_filename(a, archivePath.c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("archiveFolder() could not open archive %s for writing: %s", archivePath.c_str(), archive_error_string(a)));
	DEFER({archive_write_close(a);});

	// Open folder for reading
	struct archive* disk = archive_read_disk_new();
	DEFER({archive_read_free(disk);});
#if defined ARCH_WIN
	r = archive_read_disk_open_w(disk, string::U8toU16(folderPath).c_str());
#else
	r = archive_read_disk_open(disk, folderPath.c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("archiveFolder() could not open folder %s for reading: %s", folderPath.c_str(), archive_error_string(a)));
	DEFER({archive_read_close(a);});

	// Iterate folder
	for (;;) {
		struct archive_entry* entry = archive_entry_new();
		DEFER({archive_entry_free(entry);});

		r = archive_read_next_header2(disk, entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r < ARCHIVE_OK)
			throw Exception(string::f("archiveFolder() could not get next entry from archive: %s", archive_error_string(disk)));

		// Recurse dirs
		archive_read_disk_descend(disk);

		// Convert absolute path to relative path
		std::string entryPath;
#if defined ARCH_WIN
		entryPath = string::U16toU8(archive_entry_pathname_w(entry));
#else
		entryPath = archive_entry_pathname(entry);
#endif
		entryPath = getRelativePath(entryPath, folderPath);
#if defined ARCH_WIN
		// FIXME This doesn't seem to set UTF-8 paths on Windows.
		archive_entry_copy_pathname_w(entry, string::U8toU16(entryPath).c_str());
#else
		archive_entry_set_pathname(entry, entryPath.c_str());
#endif

		// Write file to archive
		r = archive_write_header(a, entry);
		if (r < ARCHIVE_OK)
			throw Exception(string::f("archiveFolder() could not write entry to archive: %s", archive_error_string(a)));

		// Manually copy data
#if defined ARCH_WIN
		std::string entrySourcePath = string::U16toU8(archive_entry_sourcepath_w(entry));
#else
		std::string entrySourcePath = archive_entry_sourcepath(entry);
#endif
		FILE* f = std::fopen(entrySourcePath.c_str(), "rb");
		DEFER({std::fclose(f);});
		char buf[1 << 14];
		ssize_t len;
		while ((len = std::fread(buf, 1, sizeof(buf), f)) > 0) {
			archive_write_data(a, buf, len);
		}
	}
}


void unarchiveToFolder(const std::string& archivePath, const std::string& folderPath) {
	// Based on minitar.c extract() in libarchive examples
	int r;

	// Open archive for reading
	struct archive* a = archive_read_new();
	DEFER({archive_read_free(a);});
	archive_read_support_filter_zstd(a);
	// archive_read_support_filter_all(a);
	archive_read_support_format_tar(a);
	// archive_read_support_format_all(a);
#if defined ARCH_WIN
	r = archive_read_open_filename_w(a, string::U8toU16(archivePath).c_str(), 1 << 14);
#else
	r = archive_read_open_filename(a, archivePath.c_str(), 1 << 14);
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("unzipToFolder() could not open archive %s: %s", archivePath.c_str(), archive_error_string(a)));
	DEFER({archive_read_close(a);});

	// Open folder for writing
	struct archive* disk = archive_write_disk_new();
	DEFER({archive_write_free(disk);});
	int flags = ARCHIVE_EXTRACT_TIME;
	archive_write_disk_set_options(disk, flags);
	DEFER({archive_write_close(disk);});

	// Iterate archive
	for (;;) {
		// Get next entry
		struct archive_entry* entry;
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r < ARCHIVE_OK)
			throw Exception(string::f("unzipToFolder() could not read entry from archive: %s", archive_error_string(a)));

		// Convert relative pathname to absolute based on folderPath
		std::string entryPath = archive_entry_pathname(entry);
		if (!fs::u8path(entryPath).is_relative())
			throw Exception(string::f("unzipToFolder() does not support absolute paths: %s", entryPath.c_str()));
		entryPath = fs::absolute(fs::u8path(entryPath), fs::u8path(folderPath)).u8string();
#if defined ARCH_WIN
		archive_entry_copy_pathname_w(entry, string::U8toU16(entryPath).c_str());
#else
		archive_entry_set_pathname(entry, entryPath.c_str());
#endif

		// Write entry to disk
		r = archive_write_header(disk, entry);
		if (r < ARCHIVE_OK)
			throw Exception(string::f("unzipToFolder() could not write file to folder: %s", archive_error_string(disk)));

		// Copy data to file
		for (;;) {
			const void* buf;
			size_t size;
			int64_t offset;
			// Read data from archive
			r = archive_read_data_block(a, &buf, &size, &offset);
			if (r == ARCHIVE_EOF)
				break;
			if (r < ARCHIVE_OK)
				throw Exception(string::f("unzipToFolder() could not read data from archive", archive_error_string(a)));

			// Write data to file
			r = archive_write_data_block(disk, buf, size, offset);
			if (r < ARCHIVE_OK)
				throw Exception(string::f("unzipToFolder() could not write data to file", archive_error_string(disk)));
		}

		// Close file
		r = archive_write_finish_entry(disk);
		if (r < ARCHIVE_OK)
			throw Exception(string::f("unzipToFolder() could not close file", archive_error_string(disk)));
	}
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


int64_t getNanoseconds() {
#if defined ARCH_WIN
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	// TODO Check if this is always an integer factor on all CPUs
	int64_t nsPerTick = 1000000000LL / frequency.QuadPart;
	int64_t time = counter.QuadPart * nsPerTick;
	return time;
#endif
#if defined ARCH_LIN
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	int64_t time = int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
	return time;
#endif
#if defined ARCH_MAC
	using clock = std::chrono::high_resolution_clock;
	using time_point = std::chrono::time_point<clock>;
	time_point now = clock::now();
	using duration = std::chrono::duration<int64_t, std::nano>;
	duration d = now.time_since_epoch();
	return d.count();
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
	ShellExecuteW(NULL, L"open", string::U8toU16(url).c_str(), NULL, NULL, SW_SHOWDEFAULT);
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
	ShellExecuteW(NULL, L"explore", string::U8toU16(path).c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
}


void runProcessDetached(const std::string& path) {
#if defined ARCH_WIN
	SHELLEXECUTEINFOW shExInfo;
	ZeroMemory(&shExInfo, sizeof(shExInfo));
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.lpVerb = L"runas";

	std::wstring pathW = string::U8toU16(path);
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


} // namespace system
} // namespace rack
