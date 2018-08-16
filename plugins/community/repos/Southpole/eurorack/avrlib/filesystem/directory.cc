// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// FatFS wrappers.

#include "avrlib/filesystem/directory.h"

namespace avrlib {

FilesystemStatus Directory::Open(
    const char* directory_name,
    uint16_t retry_timeout) {
  FilesystemStatus s;
  s = static_cast<FilesystemStatus>(f_opendir(&d_, directory_name));
  if (s == FS_DISK_ERROR && retry_timeout) {
    // If an open fails because of a disk access error, try to reinitialize the
    // disk access layer. This might happen because a process in the background
    // has temporarily disabled the disk access layer (for example to access
    // another device). It is OK to have the disk access layer disabled between
    // file access "sessions" -- but it is not OK to have it disabled during a
    // session.
    Filesystem::Init(retry_timeout);
    s = static_cast<FilesystemStatus>(f_opendir(&d_, directory_name));
  }
  return s;
}

FilesystemStatus Directory::Next() {
  return static_cast<FilesystemStatus>(f_readdir(&d_, &f_.file_info));
}

FilesystemStatus Directory::Rewind() {
  return static_cast<FilesystemStatus>(f_readdir(&d_, NULL));
}


}  // namespace avrlib
