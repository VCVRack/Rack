#pragma once
#include <vector>

#include <common.hpp>


namespace rack {
/** Cross-platform functions for OS, file path, and filesystem routines */
namespace system {


// Filesystem

/** Joins two paths with a directory separator.
If `path2` is an empty string, returns `path1`.
*/
std::string join(const std::string& path1, const std::string& path2 = "");
/** Join an arbitrary number of paths, from left to right. */
template <typename... Paths>
std::string join(const std::string& path1, const std::string& path2, Paths... paths) {
	return join(join(path1, path2), paths...);
}
/** Returns all entries (directories, files, symbolic links, etc) in a directory.
`depth` is the number of directories to recurse. 0 depth does not recurse. -1 depth recurses infinitely.
*/
std::vector<std::string> getEntries(const std::string& dirPath, int depth = 0);
bool exists(const std::string& path);
/** Returns whether the given path is a file. */
bool isFile(const std::string& path);
/** Returns whether the given path is a directory. */
bool isDirectory(const std::string& path);
uint64_t getFileSize(const std::string& path);
/** Moves a file or directory.
Does not overwrite the destination. If this behavior is needed, use remove() or removeRecursively() before moving.
Returns whether the rename was successful.
*/
bool rename(const std::string& srcPath, const std::string& destPath);
/** Copies a file or directory recursively.
Overwrites destination if already exists.
Returns whether the copy was successful.
*/
bool copy(const std::string& srcPath, const std::string& destPath);
/** Creates a directory.
The parent directory must exist.
Returns whether the creation was successful.
*/
bool createDirectory(const std::string& path);
/** Creates all directories up to the path.
Returns whether the creation was successful.
*/
bool createDirectories(const std::string& path);
bool createSymbolicLink(const std::string& target, const std::string& link);
/** Deletes a file or empty directory.
Returns whether the deletion was successful.
*/
bool remove(const std::string& path);
/** Deletes a file or directory recursively.
Returns the number of files and directories that were deleted.
*/
int removeRecursively(const std::string& path);
std::string getWorkingDirectory();
void setWorkingDirectory(const std::string& path);
std::string getTempDirectory();
/** Returns the absolute path beginning with "/". */
std::string getAbsolute(const std::string& path);
/** Returns the canonical (unique) path, following symlinks and "." and ".." fake directories.
The path must exist on the filesystem.
Examples:
	getCanonical("/foo/./bar/.") // "/foo/bar"
*/
std::string getCanonical(const std::string& path);
/** Extracts the parent directory of the path.
Examples:
	getDirectory("/var/tmp/example.txt") // "/var/tmp"
	getDirectory("/") // ""
	getDirectory("/var/tmp/.") // "/var/tmp"
*/
std::string getDirectory(const std::string& path);
/** Extracts the filename of the path.
Examples:
	getFilename("/foo/bar.txt") // "bar.txt"
	getFilename("/foo/.bar") // ".bar"
	getFilename("/foo/bar/") // "."
	getFilename("/foo/.") // "."
	getFilename("/foo/..") // ".."
	getFilename(".") // "."
	getFilename("..") // ".."
	getFilename("/") // "/"
*/
std::string getFilename(const std::string& path);
/** Extracts the portion of a filename without the extension.
Examples:
	getStem("/foo/bar.txt") // "bar"
	getStem("/foo/.bar") // ""
	getStem("/foo/foo.tar.ztd") // "foo.tar"
*/
std::string getStem(const std::string& path);
/** Extracts the extension of a filename, including the dot.
Examples:
	getExtension("/foo/bar.txt") // ".txt"
	getExtension("/foo/bar.") // "."
	getExtension("/foo/bar") // ""
	getExtension("/foo/bar.txt/bar.cc") // ".cc"
	getExtension("/foo/bar.txt/bar.") // "."
	getExtension("/foo/bar.txt/bar") // ""
	getExtension("/foo/.") // ""
	getExtension("/foo/..") // ""
	getExtension("/foo/.hidden") // ".hidden"
*/
std::string getExtension(const std::string& path);

// File read/write

/** Reads an entire file into a memory buffer.
Throws on error.
*/
std::vector<uint8_t> readFile(const std::string& path);
uint8_t* readFile(const std::string& path, size_t* size);

/** Writes a memory buffer to a file, overwriting if already exists.
Throws on error.
*/
void writeFile(const std::string& path, const std::vector<uint8_t>& data);

/** Compresses the contents of a directory (recursively) to an archive.
Uses the Unix Standard TAR + Zstandard format (.tar.zst).
An equivalent shell command is

	tar -c -C dirPath . | zstd -1 -o archivePath

or

	ZSTD_CLEVEL=1 tar -cf archivePath --zstd -C dirPath .

Throws on error.
*/
void archiveDirectory(const std::string& archivePath, const std::string& dirPath, int compressionLevel = 1);
std::vector<uint8_t> archiveDirectory(const std::string& dirPath, int compressionLevel = 1);

/** Extracts an archive into a directory.
An equivalent shell command is

	zstd -d < archivePath | tar -x -C dirPath

or

	tar -xf archivePath --zstd -C dirPath

As a special case, zero-byte files in the archive cause the unarchiver to delete existing files instead of overwriting them.
This is useful for removing presets in .vcvplugin packages, for example.

Throws on error.
*/
void unarchiveToDirectory(const std::string& archivePath, const std::string& dirPath);
void unarchiveToDirectory(const std::vector<uint8_t>& archiveData, const std::string& dirPath);


// Threading

/** Returns the number of logical simultaneous multithreading (SMT) (e.g. Intel Hyperthreaded) threads on the CPU. */
int getLogicalCoreCount();
/** Sets a name of the current thread for debuggers and OS-specific process viewers. */
void setThreadName(const std::string& name);

// Querying

/** Returns the caller's human-readable stack trace with "\n"-separated lines. */
std::string getStackTrace();
/** Returns the number of seconds since application launch.
Gives the most precise (fine-grained) time differences available on the OS for benchmarking purposes, while being fast to compute.
*/
double getTime();
/** Returns time since 1970-01-01 00:00:00 UTC in seconds.
*/
double getUnixTime();
double getThreadTime();
void sleep(double time);
std::string getOperatingSystemInfo();

// Applications

/** Opens a URL in a browser.
Shell injection is possible, so make sure the URL is trusted or hard coded.
Does nothing if string is blank.
Does not block.
*/
void openBrowser(const std::string& url);
/** Opens Windows Explorer, Finder, etc at a directory location.
Does nothing if string is blank.
Does not block.
*/
void openDirectory(const std::string& path);
/** Runs an executable without blocking.
The launched process will continue running if the current process is closed.
*/
void runProcessDetached(const std::string& path);

PRIVATE void init();


} // namespace system
} // namespace rack
