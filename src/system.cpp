#include <thread>
#include <regex>
#include <chrono>

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

#define ZIP_STATIC
#include <zip.h>
#include <archive.h>
#include <archive_entry.h>

#include <system.hpp>
#include <string.hpp>


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
	std::wstring pathW = string::U8toU16(path);
	_wmkdir(pathW.c_str());
#else
	mkdir(path.c_str(), 0755);
#endif
}


void createDirectories(const std::string& path) {
	for (size_t i = 1; i < path.size(); i++) {
		char c = path[i];
		if (c == '/' || c == '\\')
			createDirectory(path.substr(0, i));
	}
	createDirectory(path);
}


void removeDirectory(const std::string& path) {
#if defined ARCH_WIN
	std::wstring pathW = string::U8toU16(path);
	_wrmdir(pathW.c_str());
#else
	rmdir(path.c_str());
#endif
}


void removeDirectories(const std::string& path) {
	removeDirectory(path);
	for (size_t i = path.size() - 1; i >= 1; i--) {
		char c = path[i];
		if (c == '/' || c == '\\')
			removeDirectory(path.substr(0, i));
	}
}


std::string getWorkingDirectory() {
#if defined ARCH_WIN
	wchar_t buf[4096] = L"";
	GetCurrentDirectory(sizeof(buf), buf);
	return string::U16toU8(buf);
#else
	char buf[4096] = "";
	getcwd(buf, sizeof(buf));
	return buf;
#endif
}


void setWorkingDirectory(const std::string& path) {
#if defined ARCH_WIN
	std::wstring pathW = string::U8toU16(path);
	SetCurrentDirectory(pathW.c_str());
#else
	chdir(path.c_str());
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
	std::wstring urlW = string::U8toU16(url);
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
	std::wstring pathW = string::U8toU16(path);
	ShellExecuteW(NULL, L"explore", pathW.c_str(), NULL, NULL, SW_SHOWDEFAULT);
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


/** Behaves like `std::filesystem::relative()`.
Limitation: `p` must be a descendant of `base`. Doesn't support adding `../` to the return path.
*/
static filesystem::path getRelativePath(filesystem::path p, filesystem::path base = filesystem::current_path()) {
	p = filesystem::absolute(p);
	base = filesystem::absolute(base);
	std::string pStr = p.generic_u8string();
	std::string baseStr = base.generic_u8string();
	if (pStr.size() < baseStr.size())
		throw Exception("getRelativePath() error: path is shorter than base");
	if (!std::equal(baseStr.begin(), baseStr.end(), pStr.begin()))
		throw Exception("getRelativePath() error: path does not begin with base");

	// If p == base, this correctly returns "."
	return "." + std::string(pStr.begin() + baseStr.size(), pStr.end());
}


void archiveFolder(const filesystem::path& archivePath, const filesystem::path& folderPath){
	// Based on minitar.c create() in libarchive examples
	int r;

	// Open archive for writing
	struct archive* a = archive_write_new();
	DEFER({archive_write_free(a);});
	archive_write_set_format_ustar(a);
	archive_write_add_filter_zstd(a);
#if defined ARCH_WIN
	r = archive_write_open_filename_w(a, archivePath.generic_wstring().c_str());
#else
	r = archive_write_open_filename(a, archivePath.generic_u8string().c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("archiveFolder() could not open archive %s for writing: %s", archivePath.generic_u8string().c_str(), archive_error_string(a)));
	DEFER({archive_write_close(a);});

	// Open folder for reading
	struct archive* disk = archive_read_disk_new();
	DEFER({archive_read_free(disk);});
#if defined ARCH_WIN
	r = archive_read_disk_open_w(disk, folderPath.generic_wstring().c_str());
#else
	r = archive_read_disk_open(disk, folderPath.generic_u8string().c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("archiveFolder() could not open folder %s for reading: %s", folderPath.generic_u8string().c_str(), archive_error_string(a)));
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
		filesystem::path entryPath =
#if defined ARCH_WIN
			archive_entry_pathname_w(entry);
#else
			archive_entry_pathname(entry);
#endif
		entryPath = getRelativePath(entryPath, folderPath);
#if defined ARCH_WIN
		archive_entry_copy_pathname_w(entry, entryPath.generic_wstring().c_str());
#else
		archive_entry_set_pathname(entry, entryPath.generic_u8string().c_str());
#endif

		// Write file to archive
		r = archive_write_header(a, entry);
		if (r < ARCHIVE_OK)
			throw Exception(string::f("archiveFolder() could not write entry to archive: %s", archive_error_string(a)));

		// Manually copy data
#if defined ARCH_WIN
		filesystem::path entrySourcePath = archive_entry_sourcepath_w(entry);
#else
		filesystem::path entrySourcePath = archive_entry_sourcepath(entry);
#endif
		FILE* f = std::fopen(entrySourcePath.generic_u8string().c_str(), "rb");
		DEFER({std::fclose(f);});
		char buf[1 << 14];
		ssize_t len;
		while ((len = std::fread(buf, 1, sizeof(buf), f)) > 0) {
			archive_write_data(a, buf, len);
		}
	}
}


void unarchiveToFolder(const filesystem::path& archivePath, const filesystem::path& folderPath) {
	// Based on minitar.c extract() in libarchive examples
	int r;

	// Open archive for reading
	struct archive* a = archive_read_new();
	DEFER({archive_read_free(a);});
	archive_read_support_filter_zstd(a);
	// archive_read_support_filter_all(a);
	archive_read_support_format_tar(a);
	// archive_read_support_format_all(a);
	DEBUG("opening %s %s", archivePath.generic_string().c_str(), archivePath.string().c_str());
#if defined ARCH_WIN
	r = archive_read_open_filename_w(a, archivePath.generic_wstring().c_str(), 1 << 14);
#else
	r = archive_read_open_filename(a, archivePath.generic_u8string().c_str(), 1 << 14);
#endif
	if (r < ARCHIVE_OK)
		throw Exception(string::f("unzipToFolder() could not open archive %s: %s", archivePath.generic_u8string().c_str(), archive_error_string(a)));
	DEFER({archive_read_close(a);});

	// Open folder for writing
	struct archive* disk = archive_write_disk_new();
	DEFER({archive_write_free(disk);});
	int flags = ARCHIVE_EXTRACT_TIME;
	archive_write_disk_set_options(disk, flags);
	// archive_write_disk_set_standard_lookup(disk);
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
		filesystem::path entryPath = filesystem::u8path(archive_entry_pathname(entry));
		if (!entryPath.is_relative())
			throw Exception(string::f("unzipToFolder() does not support absolute paths: %s", entryPath.generic_u8string().c_str()));
		entryPath = filesystem::absolute(entryPath, folderPath);
#if defined ARCH_WIN
		archive_entry_copy_pathname_w(entry, entryPath.generic_wstring().c_str());
#else
		archive_entry_set_pathname(entry, entryPath.generic_u8string().c_str());
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


} // namespace system
} // namespace rack
