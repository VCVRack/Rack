#pragma once
#include "common.hpp"
#include <list>


namespace rack {
namespace system {


std::list<std::string> listEntries(const std::string &path);
bool isFile(const std::string &path);
bool isDirectory(const std::string &path);
void copyFile(const std::string &srcPath, const std::string &destPath);
void createDirectory(const std::string &path);

/** Currently this lies and returns the number of logical cores instead. */
int getPhysicalCoreCount();
void setThreadName(const std::string &name);
void setThreadRealTime();
std::string getStackTrace();

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void openBrowser(const std::string &url);


} // namespace system
} // namespace rack
