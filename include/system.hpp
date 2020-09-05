#pragma once
#include <list>

#include <common.hpp>
#include <experimental/filesystem>


namespace rack {


// In C++17, this will be `std::filesystem`
namespace filesystem = std::experimental::filesystem;


/** Cross-platform functions for operating systems routines
*/
namespace system {


/** Returns a list of all entries (directories, files, symbols) in a directory.
Sorted alphabetically.
*/
std::list<std::string> getEntries(const std::string& path);
std::list<std::string> getEntriesRecursive(const std::string &path, int depth);
/** Returns whether the given path is a file. */
bool isFile(const std::string& path);
/** Returns whether the given path is a directory. */
bool isDirectory(const std::string& path);
/** Moves a file. */
void moveFile(const std::string& srcPath, const std::string& destPath);
/** Copies a file. */
void copyFile(const std::string& srcPath, const std::string& destPath);
/** Creates a directory.
The parent directory must exist.
*/
void createDirectory(const std::string& path);
/** Creates all directories up to the path.
*/
void createDirectories(const std::string& path);
/** Deletes a directory.
The directory must be empty. Fails silently.
*/
void removeDirectory(const std::string& path);
/** Deletes a directory if empty and all parent directories that are then empty.
*/
void removeDirectories(const std::string& path);
std::string getWorkingDirectory();
void setWorkingDirectory(const std::string& path);
/** Returns the number of logical simultaneous multithreading (SMT) (e.g. Intel Hyperthreaded) threads on the CPU. */
int getLogicalCoreCount();
/** Sets a name of the current thread for debuggers and OS-specific process viewers. */
void setThreadName(const std::string& name);
/** Returns the caller's human-readable stack trace with "\n"-separated lines. */
std::string getStackTrace();
/** Returns the current number of nanoseconds since the epoch.
The goal of this function is to give the most precise (fine-grained) time available on the OS for benchmarking purposes, while being fast to compute.
The epoch is undefined. Do not use this function to get absolute time, as it is different on each OS.
*/
int64_t getNanoseconds();
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
std::string getOperatingSystemInfo();
/** Compresses the contents of a folder (recursively) to an archive.
Currently supports the "ustar zstd" format (.tar.zst)
An equivalent shell command is

	tar -cf archivePath --zstd -C folderPath .
*/
void archiveFolder(const filesystem::path& archivePath, const filesystem::path& folderPath);
/** Extracts an archive into a folder.
An equivalent shell command is

	tar -xf archivePath --zstd -C folderPath
*/
void unarchiveToFolder(const filesystem::path& archivePath, const filesystem::path& folderPath);


} // namespace system
} // namespace rack
