#pragma once
#include <vector>
#include "common.hpp"


namespace rack {
namespace system {


std::vector<std::string> listEntries(std::string path);
bool isFile(std::string path);
bool isDirectory(std::string path);
void copyFile(std::string srcPath, std::string destPath);
void createDirectory(std::string path);

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void openBrowser(std::string url);


} // namespace system
} // namespace rack
