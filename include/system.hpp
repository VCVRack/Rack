#pragma once
#include "common.hpp"
#include <list>


namespace rack {


/** Cross-platform functions for operating systems routines
*/
namespace system {


/** Returns a list of all entries (directories, files, symbols) in a directory. */
std::list<std::string> listEntries(const std::string &path);
/** Returns whether the given path is a file. */
bool isFile(const std::string &path);
/** Returns whether the given path is a directory. */
bool isDirectory(const std::string &path);
/** Copies a file. */
void copyFile(const std::string &srcPath, const std::string &destPath);
/** Creates a directory.
The parent directory must exist.
*/
void createDirectory(const std::string &path);
/** Returns the number of logical simultaneous multithreading (SMT) (e.g. Intel Hyperthreaded) threads on the CPU. */
int getLogicalCoreCount();
/** Sets a name of the current thread for debuggers and OS-specific process viewers. */
void setThreadName(const std::string &name);
/** Sets the current thread to be high-priority. */
void setThreadRealTime();
/** Returns the caller's human-readable stack trace with "\n"-separated lines. */
std::string getStackTrace();
/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void openBrowser(const std::string &url);
void openFolder(const std::string &path);


} // namespace system
} // namespace rack
