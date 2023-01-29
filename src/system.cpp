#include <thread>
#include <regex>
#include <chrono>
#include <ghc/filesystem.hpp>

#include <dirent.h>
#include <sys/stat.h>
#include <cxxabi.h> // for abi::__cxa_demangle

#if defined ARCH_LIN || defined ARCH_MAC
	#include <pthread.h>
	#include <time.h> // for clock_gettime
	#include <sched.h>
	#include <execinfo.h> // for backtrace and backtrace_symbols
	#include <unistd.h> // for execl
	#include <sys/utsname.h>
#endif

#if defined ARCH_MAC
	#include <mach/mach_init.h>
	#include <mach/thread_act.h>
	#include <mach/mach_time.h>
	#include <sys/sysctl.h>
#endif

#if defined ARCH_WIN
	#include <windows.h>
	#include <shellapi.h>
	#include <processthreadsapi.h>
	#include <dbghelp.h>
#endif

#include <archive.h>
#include <archive_entry.h>
#if defined ARCH_MAC
	#include <locale.h>
#endif

#include <system.hpp>
#include <string.hpp>


/*
In C++17, this will be `std::filesystem`
Important: When using `fs::path`, always convert strings to/from UTF-8 using

	fs::path p = fs::u8path(s);
	std::string s = p.generic_u8string();
*/
namespace fs = ghc::filesystem;


namespace rack {
namespace system {


std::string join(const std::string& path1, const std::string& path2) {
	return (fs::u8path(path1) / fs::u8path(path2)).generic_u8string();
}


static void appendEntries(std::vector<std::string>& entries, const fs::path& dir, int depth) {
	try {
		for (const auto& entry : fs::directory_iterator(dir)) {
			entries.push_back(entry.path().generic_u8string());
			// Recurse if depth > 0 (limited recursion) or depth < 0 (infinite recursion).
			if (depth != 0) {
				if (entry.is_directory()) {
					appendEntries(entries, entry.path(), depth - 1);
				}
			}
		}
	}
	catch (fs::filesystem_error& e) {}
}


std::vector<std::string> getEntries(const std::string& dirPath, int depth) {
	std::vector<std::string> entries;
	appendEntries(entries, fs::u8path(dirPath), depth);
	return entries;
}


bool exists(const std::string& path) {
	try {
		return fs::exists(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool isFile(const std::string& path) {
	try {
		return fs::is_regular_file(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool isDirectory(const std::string& path) {
	try {
		return fs::is_directory(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


uint64_t getFileSize(const std::string& path) {
	try {
		return fs::file_size(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return 0;
	}
}


bool rename(const std::string& srcPath, const std::string& destPath) {
	try {
		fs::rename(fs::u8path(srcPath), fs::u8path(destPath));
		return true;
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool copy(const std::string& srcPath, const std::string& destPath) {
	try {
		fs::copy(fs::u8path(srcPath), fs::u8path(destPath), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		return true;
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool createDirectory(const std::string& path) {
	try {
		return fs::create_directory(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool createDirectories(const std::string& path) {
	try {
		return fs::create_directories(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool createSymbolicLink(const std::string& target, const std::string& link) {
	try {
		fs::create_symlink(fs::u8path(target), fs::u8path(link));
		return true;
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


bool remove(const std::string& path) {
	try {
		return fs::remove(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		return false;
	}
}


int removeRecursively(const std::string& pathStr) {
	fs::path path = fs::u8path(pathStr);
	try {
		// Make all entries writable before attempting to remove
		for (auto& entry : fs::recursive_directory_iterator(path)) {
			fs::permissions(entry.path(), fs::perms::owner_write, fs::perm_options::add);
		}

		return fs::remove_all(path);
	}
	catch (fs::filesystem_error& e) {
		return 0;
	}
}


std::string getWorkingDirectory() {
	try {
		return fs::current_path().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


void setWorkingDirectory(const std::string& path) {
	try {
		fs::current_path(fs::u8path(path));
	}
	catch (fs::filesystem_error& e) {
		// Do nothing
	}
}


std::string getTempDirectory() {
	try {
		return fs::temp_directory_path().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getAbsolute(const std::string& path) {
	try {
		return fs::absolute(fs::u8path(path)).generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getCanonical(const std::string& path) {
	try {
		return fs::canonical(fs::u8path(path)).generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getDirectory(const std::string& path) {
	try {
		return fs::u8path(path).parent_path().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getFilename(const std::string& path) {
	try {
		return fs::u8path(path).filename().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getStem(const std::string& path) {
	try {
		return fs::u8path(path).stem().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::string getExtension(const std::string& path) {
	try {
		return fs::u8path(path).extension().generic_u8string();
	}
	catch (fs::filesystem_error& e) {
		return "";
	}
}


std::vector<uint8_t> readFile(const std::string& path) {
	std::vector<uint8_t> data;
	FILE* f = std::fopen(path.c_str(), "rb");
	if (!f)
		throw Exception("Cannot read file %s", path.c_str());
	DEFER({
		std::fclose(f);
	});

	// Get file size so we can make a single allocation
	std::fseek(f, 0, SEEK_END);
	size_t len = std::ftell(f);
	std::fseek(f, 0, SEEK_SET);

	data.resize(len);
	std::fread(data.data(), 1, len, f);
	return data;
}


uint8_t* readFile(const std::string& path, size_t* size) {
	FILE* f = std::fopen(path.c_str(), "rb");
	if (!f)
		throw Exception("Cannot read file %s", path.c_str());
	DEFER({
		std::fclose(f);
	});

	// Get file size so we can make a single allocation
	std::fseek(f, 0, SEEK_END);
	size_t len = std::ftell(f);
	std::fseek(f, 0, SEEK_SET);

	uint8_t* data = (uint8_t*) std::malloc(len);
	std::fread(data, 1, len, f);
	if (size)
		*size = len;
	return data;
}


void writeFile(const std::string& path, const std::vector<uint8_t>& data) {
	FILE* f = std::fopen(path.c_str(), "wb");
	if (!f)
		throw Exception("Cannot create file %s", path.c_str());
	DEFER({
		std::fclose(f);
	});

	std::fwrite(data.data(), 1, data.size(), f);
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
		throw Exception("%s", e.what());
	}
	if (path.size() < base.size())
		throw Exception("getRelativePath() error: path is shorter than base");
	if (!std::equal(base.begin(), base.end(), path.begin()))
		throw Exception("getRelativePath() error: path does not begin with base");

	// If path == base, this correctly returns "."
	return "." + std::string(path.begin() + base.size(), path.end());
}


static la_ssize_t archiveWriteVectorCallback(struct archive* a, void* client_data, const void* buffer, size_t length) {
	assert(client_data);
	std::vector<uint8_t>& data = *((std::vector<uint8_t>*) client_data);
	uint8_t* buf = (uint8_t*) buffer;
	data.insert(data.end(), buf, buf + length);
	return length;
}


static void archiveDirectory(const std::string& archivePath, std::vector<uint8_t>* archiveData, const std::string& dirPath, int compressionLevel) {
	// Based on minitar.c create() in libarchive examples
	int r;

	// Open archive for writing
	struct archive* a = archive_write_new();
	DEFER({archive_write_free(a);});
	// For some reason libarchive adds 10k of padding to archive_write_open() (but not archive_write_open_filename()) unless this is set to 0.
	archive_write_set_bytes_per_block(a, 0);
	archive_write_set_format_pax_restricted(a);
	archive_write_add_filter_zstd(a);
	if (!(0 <= compressionLevel && compressionLevel <= 19))
		throw Exception("Invalid Zstandard compression level");
	r = archive_write_set_filter_option(a, NULL, "compression-level", std::to_string(compressionLevel).c_str());
	if (r < ARCHIVE_OK)
		throw Exception("Archiver could not set filter option: %s", archive_error_string(a));

	if (archiveData) {
		// Open vector
		archive_write_open(a, (void*) archiveData, NULL, archiveWriteVectorCallback, NULL);
	}
	else {
		// Open file
#if defined ARCH_WIN
		r = archive_write_open_filename_w(a, string::UTF8toUTF16(archivePath).c_str());
#else
		r = archive_write_open_filename(a, archivePath.c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception("Archiver could not open archive %s for writing: %s", archivePath.c_str(), archive_error_string(a));
	}
	DEFER({archive_write_close(a);});

	// Open dir for reading
	struct archive* disk = archive_read_disk_new();
	DEFER({archive_read_free(disk);});
#if defined ARCH_WIN
	r = archive_read_disk_open_w(disk, string::UTF8toUTF16(dirPath).c_str());
#else
	r = archive_read_disk_open(disk, dirPath.c_str());
#endif
	if (r < ARCHIVE_OK)
		throw Exception("Archiver could not open dir %s for reading: %s", dirPath.c_str(), archive_error_string(a));
	DEFER({archive_read_close(a);});

	// Iterate dir
	for (;;) {
		struct archive_entry* entry = archive_entry_new();
		DEFER({archive_entry_free(entry);});

		r = archive_read_next_header2(disk, entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r < ARCHIVE_OK)
			throw Exception("Archiver could not get next entry from archive: %s", archive_error_string(disk));

		// Recurse dirs
		archive_read_disk_descend(disk);

		// Convert absolute path to relative path
		std::string entryPath;
#if defined ARCH_WIN
		entryPath = string::UTF16toUTF8(archive_entry_pathname_w(entry));
#else
		entryPath = archive_entry_pathname(entry);
#endif

		entryPath = getRelativePath(entryPath, dirPath);

#if defined ARCH_WIN
		// FIXME This doesn't seem to set UTF-8 paths on Windows.
		archive_entry_copy_pathname_w(entry, string::UTF8toUTF16(entryPath).c_str());
#else
		archive_entry_set_pathname(entry, entryPath.c_str());
#endif

		// Set uid and gid to 0 because we don't need to store these.
		archive_entry_set_uid(entry, 0);
		archive_entry_set_uname(entry, NULL);
		archive_entry_set_gid(entry, 0);
		archive_entry_set_gname(entry, NULL);

		// Write file to archive
		r = archive_write_header(a, entry);
		if (r < ARCHIVE_OK)
			throw Exception("Archiver could not write entry to archive: %s", archive_error_string(a));

		// Manually copy data
#if defined ARCH_WIN
		std::string entrySourcePath = string::UTF16toUTF8(archive_entry_sourcepath_w(entry));
#else
		std::string entrySourcePath = archive_entry_sourcepath(entry);
#endif
		FILE* f = std::fopen(entrySourcePath.c_str(), "rb");
		DEFER({std::fclose(f);});
		char buf[1 << 16];
		ssize_t len;
		while ((len = std::fread(buf, 1, sizeof(buf), f)) > 0) {
			archive_write_data(a, buf, len);
		}
	}
}

void archiveDirectory(const std::string& archivePath, const std::string& dirPath, int compressionLevel) {
	archiveDirectory(archivePath, NULL, dirPath, compressionLevel);
}

std::vector<uint8_t> archiveDirectory(const std::string& dirPath, int compressionLevel) {
	std::vector<uint8_t> archiveData;
	archiveDirectory("", &archiveData, dirPath, compressionLevel);
	return archiveData;
}


struct ArchiveReadVectorData {
	const std::vector<uint8_t>* data = NULL;
	size_t pos = 0;
};

static la_ssize_t archiveReadVectorCallback(struct archive *a, void* client_data, const void** buffer) {
	assert(client_data);
	ArchiveReadVectorData* arvd = (ArchiveReadVectorData*) client_data;
	assert(arvd->data);
	const std::vector<uint8_t>& data = *arvd->data;
	*buffer = &data[arvd->pos];
	// Read up to some block size of bytes
	size_t len = std::min(data.size() - arvd->pos, size_t(1 << 16));
	arvd->pos += len;
	return len;
}

static void unarchiveToDirectory(const std::string& archivePath, const std::vector<uint8_t>* archiveData, const std::string& dirPathStr) {
#if defined ARCH_MAC
	// libarchive depends on locale so set thread locale
	// If locale is not found, returns NULL which resets thread to global locale
	locale_t loc = newlocale(LC_CTYPE_MASK, "en_US.UTF-8", NULL);
	locale_t oldLoc = uselocale(loc);
	freelocale(loc);
	DEFER({
		uselocale(oldLoc);
	});
#endif

	fs::path dirPath = fs::u8path(dirPathStr);

	// Based on minitar.c extract() in libarchive examples
	int r;

	// Open archive for reading
	struct archive* a = archive_read_new();
	if (!a)
		throw Exception("Unarchiver could not be created");
	DEFER({archive_read_free(a);});
	archive_read_support_filter_zstd(a);
	// archive_read_support_filter_all(a);
	archive_read_support_format_tar(a);
	// archive_read_support_format_all(a);

	ArchiveReadVectorData arvd;
	if (archiveData) {
		// Open vector
		arvd.data = archiveData;
		archive_read_open(a, &arvd, NULL, archiveReadVectorCallback, NULL);
	}
	else {
		// Open file
		const size_t blockSize = 1 << 16;
#if defined ARCH_WIN
		r = archive_read_open_filename_w(a, string::UTF8toUTF16(archivePath).c_str(), blockSize);
#else
		r = archive_read_open_filename(a, archivePath.c_str(), blockSize);
#endif
		if (r < ARCHIVE_OK)
			throw Exception("Unarchiver could not open archive %s: %s", archivePath.c_str(), archive_error_string(a));
	}
	DEFER({archive_read_close(a);});

	// Open dir for writing
	struct archive* disk = archive_write_disk_new();
	DEFER({archive_write_free(disk);});
	// Don't restore timestamps
	int flags = 0;
	// Delete existing files instead of truncating and rewriting
	flags |= ARCHIVE_EXTRACT_UNLINK;
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
			throw Exception("Unarchiver could not read entry from archive: %s", archive_error_string(a));

		// Convert relative pathname to absolute based on dirPath
		fs::path entryPath = fs::u8path(archive_entry_pathname(entry));
		// DEBUG("entryPath: %s", entryPath.generic_u8string().c_str());
		if (!entryPath.is_relative())
			throw Exception("Unarchiver does not support absolute tar paths: %s", entryPath.u8string().c_str());

		entryPath = dirPath / entryPath;
#if defined ARCH_WIN
		archive_entry_copy_pathname_w(entry, string::UTF8toUTF16(entryPath.u8string()).c_str());
#else
		archive_entry_set_pathname(entry, entryPath.u8string().c_str());
#endif

		mode_t mode = archive_entry_mode(entry);
		mode_t filetype = archive_entry_filetype(entry);
		int64_t size = archive_entry_size(entry);

		// Force minimum modes
		if (filetype == AE_IFREG) {
			mode |= 0644;
		}
		else if (filetype == AE_IFDIR) {
			mode |= 0755;
		}
		archive_entry_set_mode(entry, mode);

		// Delete zero-byte files
		if (filetype == AE_IFREG && size == 0) {
			remove(entryPath.generic_u8string());
			continue;
		}

		// Write entry to disk
		r = archive_write_header(disk, entry);
		if (r < ARCHIVE_OK)
			throw Exception("Unarchiver could not write file to dir: %s", archive_error_string(disk));

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
				throw Exception("Unarchiver could not read data from archive: %s", archive_error_string(a));

			// Write data to file
			r = archive_write_data_block(disk, buf, size, offset);
			if (r < ARCHIVE_OK)
				throw Exception("Unarchiver could not write data to file: %s", archive_error_string(disk));
		}

		// Close file
		r = archive_write_finish_entry(disk);
		if (r < ARCHIVE_OK)
			throw Exception("Unarchiver could not close file: %s", archive_error_string(disk));
	}
}

void unarchiveToDirectory(const std::string& archivePath, const std::string& dirPath) {
	unarchiveToDirectory(archivePath, NULL, dirPath);
}

void unarchiveToDirectory(const std::vector<uint8_t>& archiveData, const std::string& dirPath) {
	unarchiveToDirectory("", &archiveData, dirPath);
}


int getLogicalCoreCount() {
	return std::thread::hardware_concurrency();
}


void setThreadName(const std::string& name) {
#if defined ARCH_LIN
	pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#elif defined ARCH_MAC
	// Not supported (yet) on Mac
#elif defined ARCH_WIN
	// Not supported on Windows
#endif
}


std::string getStackTrace() {
	void* stack[128];
	int stackLen = LENGTHOF(stack);
	std::string s;

#if defined ARCH_LIN || defined ARCH_MAC
	stackLen = backtrace(stack, stackLen);
	char** strings = backtrace_symbols(stack, stackLen);
	if (!strings)
		return "";

	// Skip the first line because it's this function.
	for (int i = 1; i < stackLen; i++) {
		s += string::f("%d: ", stackLen - i - 1);
		std::string line = strings[i];
#if ARCH_LIN
		// Parse line, e.g.
		// ./main(__mangled_symbol+0x100) [0x12345678]
		std::regex r(R"((.+?)\((.*?)\+(.+?)\) (.+?))");
		std::smatch match;
		if (std::regex_match(line, match, r)) {
			s += match[1].str();
			s += "(";
			std::string symbol = match[2].str();
			// Demangle symbol
			char* symbolD = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, NULL);
			if (symbolD) {
				symbol = symbolD;
				free(symbolD);
			}
			s += symbol;
			s += "+";
			s += match[3].str();
			s += ")";
		}
		else {
			// If regex fails, just use the raw line
			s += line;
		}
#elif ARCH_MAC
		// Parse line, e.g.
		// 1   Rack                                0x0000000002ddc3eb _mangled_symbol + 27
		std::regex r(R"((\d+)\s+(.+?)\s+(.+?) (.*?) \+ (.+?))");
		std::smatch match;
		if (std::regex_match(line, match, r)) {
			s += match[2].str();
			s += "(";
			std::string symbol = match[4].str();
			// Demangle symbol
			char* symbolD = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, NULL);
			if (symbolD) {
				symbol = symbolD;
				free(symbolD);
			}
			s += symbol;
			s += "+";
			s += match[5].str();
			s += ")";
		}
		else {
			// If regex fails, just use the raw line
			s += line;
		}
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
		s += string::f("%d: %s 0x%" PRIx64 "\n", stackLen - i - 1, symbol->Name, symbol->Address);
	}
	free(symbol);
#endif

	return s;
}


#if defined ARCH_WIN
	static int64_t startCounter = 0;
	static double counterTime = 0.0;
#endif
#if defined ARCH_LIN
	static int64_t startTime = 0;
#endif
#if defined ARCH_MAC
	static int64_t startCounter = 0;
	static double counterTime = 0.0;
#endif

static void initTime() {
#if defined ARCH_WIN
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	startCounter = counter.QuadPart;

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	counterTime = 1.0 / frequency.QuadPart;
#endif
#if defined ARCH_LIN
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	startTime = int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#endif
#if defined ARCH_MAC
	startCounter = mach_absolute_time();

	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	counterTime = 1e-9 * (double) tb.numer / tb.denom;
#endif
}


double getTime() {
#if defined ARCH_WIN
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (counter.QuadPart - startCounter) * counterTime;
#endif
#if defined ARCH_LIN
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	int64_t time = int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
	return (time - startTime) / 1e9;
#endif
#if defined ARCH_MAC
	int64_t counter = mach_absolute_time();
	return (counter - startCounter) * counterTime;
#endif
}


double getUnixTime() {
	// This is not guaranteed to return the time since 1970 in C++11. (It only does in C++20).
	// However, it does on all platforms I care about.
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration<double>(duration).count();
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
	FILETIME creationTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;
	if (GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime) == 0)
		return 0.0;
	return ((uint64_t(userTime.dwHighDateTime) << 32) + userTime.dwLowDateTime) * 1e-7;
#endif
}


void sleep(double time) {
	std::this_thread::sleep_for(std::chrono::duration<double>(time));
}


std::string getOperatingSystemInfo() {
#if defined ARCH_LIN
	struct utsname u;
	uname(&u);
	return string::f("%s %s %s %s", u.sysname, u.release, u.version, u.machine);
#elif defined ARCH_MAC
	// From https://opensource.apple.com/source/cctools/cctools-973.0.1/libstuff/macosx_deployment_target.c.auto.html
	char osversion[32];
	int osversion_name[2] = {CTL_KERN, KERN_OSRELEASE};
	size_t osversion_len = sizeof(osversion) - 1;
	if (sysctl(osversion_name, 2, osversion, &osversion_len, NULL, 0) != 0)
		return "Mac";

	int major = 0;
	int minor = 0;
	if (sscanf(osversion, "%d.%d", &major, &minor) != 2)
		return "Mac";

	// Try to match version numbers to retail versions
	if (major >= 20) {
		if (major >= 22) {
			minor -= 1;
		}
		major -= 9;
		return string::f("Mac %d.%d", major, minor);
	}
	else {
		major -= 4;
		return string::f("Mac 10.%d.%d", major, minor);
	}
#elif defined ARCH_WIN
	OSVERSIONINFOW info;
	ZeroMemory(&info, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionExW(&info);
	// See https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_osversioninfoa for a list of Windows version numbers.
	return string::f("Windows %lu.%lu", info.dwMajorVersion, info.dwMinorVersion);
#endif
}


void openBrowser(const std::string& url) {
	if (url.empty())
		return;

	std::string urlL = url;
	std::thread t([=] {
#if defined ARCH_LIN
		std::string command = "xdg-open \"" + urlL + "\"";
		(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
		std::string command = "open \"" + urlL + "\"";
		std::system(command.c_str());
#endif
#if defined ARCH_WIN
		ShellExecuteW(NULL, L"open", string::UTF8toUTF16(urlL).c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
	});
	t.detach();
}


void openDirectory(const std::string& path) {
	if (path.empty())
		return;

	std::string pathL = path;
	std::thread t([=] {
#if defined ARCH_LIN
		std::string command = "xdg-open \"" + pathL + "\"";
		(void) std::system(command.c_str());
#endif
#if defined ARCH_MAC
		std::string command = "open \"" + pathL + "\"";
		std::system(command.c_str());
#endif
#if defined ARCH_WIN
		ShellExecuteW(NULL, L"explore", string::UTF8toUTF16(pathL).c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
	});
	t.detach();
}


void runProcessDetached(const std::string& path) {
#if defined ARCH_WIN
	SHELLEXECUTEINFOW shExInfo;
	ZeroMemory(&shExInfo, sizeof(shExInfo));
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.lpVerb = L"runas";

	std::wstring pathW = string::UTF8toUTF16(path);
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


void init() {
	initTime();
}


} // namespace system
} // namespace rack
