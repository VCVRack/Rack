#pragma once
#include <list>

#include <common.hpp>


namespace rack {


/** Cross-platform functions for operating systems routines
*/
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
/** Returns a list of all entries (directories, files, symbolic links, etc) in a directory.
`depth` is the number of directories to recurse. 0 depth does not recurse. -1 depth recurses infinitely.
*/
std::list<std::string> getEntries(const std::string& dirPath, int depth = 0);
bool doesExist(const std::string& path);
/** Returns whether the given path is a file. */
bool isFile(const std::string& path);
/** Returns whether the given path is a directory. */
bool isDirectory(const std::string& path);
uint64_t getFileSize(const std::string& path);
/** Moves a file or folder.
Does not overwrite the destination. If this behavior is needed, use remove() or removeRecursively() before moving.
*/
void rename(const std::string& srcPath, const std::string& destPath);
/** Copies a file or folder recursively. */
void copy(const std::string& srcPath, const std::string& destPath);
/** Creates a directory.
The parent directory must exist.
*/
bool createDirectory(const std::string& path);
/** Creates all directories up to the path.
*/
bool createDirectories(const std::string& path);
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
std::string getTempDir();
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
	getExtension("/foo/bar.txt") // "bar"
	getExtension("/foo/.bar") // ""
	getExtension("/foo/foo.bar.baz.tar") // "foo.bar.baz"
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

/** Compresses the contents of a folder (recursively) to an archive.
Currently supports the "ustar zstd" format (.tar.zst)
An equivalent shell command is

	tar -cf archivePath --zstd -C folderPath .
*/
void archiveFolder(const std::string& archivePath, const std::string& folderPath);
/** Extracts an archive into a folder.
An equivalent shell command is

	tar -xf archivePath --zstd -C folderPath
*/
void unarchiveToFolder(const std::string& archivePath, const std::string& folderPath);


// Threading

/** Returns the number of logical simultaneous multithreading (SMT) (e.g. Intel Hyperthreaded) threads on the CPU. */
int getLogicalCoreCount();
/** Sets a name of the current thread for debuggers and OS-specific process viewers. */
void setThreadName(const std::string& name);

// Querying

/** Returns the caller's human-readable stack trace with "\n"-separated lines. */
std::string getStackTrace();
/** Returns the current number of nanoseconds since the epoch.
The goal of this function is to give the most precise (fine-grained) time available on the OS for benchmarking purposes, while being fast to compute.
The epoch is undefined. Do not use this function to get absolute time, as it is different on each OS.
*/
int64_t getNanoseconds();
std::string getOperatingSystemInfo();

// Applications

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void openBrowser(const std::string& url);
/** Opens Windows Explorer, Finder, etc at the folder location. */
void openFolder(const std::string& path);
/** Runs an executable without blocking.
The launched process will continue running if the current process is closed.
*/
void runProcessDetached(const std::string& path);


} // namespace system
} // namespace rack
