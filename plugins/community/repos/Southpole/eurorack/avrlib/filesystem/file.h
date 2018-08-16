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

#ifndef AVRLIB_FILESYSTEM_FILE_H_
#define AVRLIB_FILESYSTEM_FILE_H_

#include <string.h>

#include "avrlib/avrlib.h"

#include "avrlib/filesystem/filesystem.h"

namespace avrlib {

enum FileAttributes {
  FS_READ = FA_READ,
  FS_WRITE = FA_WRITE,
  FS_OPEN_EXISTING = FA_OPEN_EXISTING,
  FS_OPEN_ALWAYS = FA_OPEN_ALWAYS,
  FS_CREATE_NEW = FA_CREATE_NEW,
  FS_CREATE_ALWAYS = FA_CREATE_ALWAYS
};

class File {
 public:
  File();
  ~File();
  
  FilesystemStatus Open(const char* file_name, const char* mode) {
    return Open(file_name, mode, 0);
  }
  
  FilesystemStatus Open(const char* file_name, uint8_t attributes) {
    return Open(file_name, attributes, 0);
  }
  
  FilesystemStatus Open(
      const char* file_name,
      const char* mode,
      uint16_t retry_timeout);
  FilesystemStatus Open(
      const char* file_name,
      uint8_t attributes,
      uint16_t retry_timeout);  
  
  FilesystemStatus Seek(uint32_t position);
  FilesystemStatus Close();
  FilesystemStatus Truncate();
  FilesystemStatus Sync();
  FilesystemStatus Read(uint8_t* data, uint16_t size, uint16_t* read);
  FilesystemStatus Write(
      const uint8_t* data,
      uint16_t size,
      uint16_t* written);
  
  FilesystemStatus Write(
      const char* data,
      uint16_t size,
      uint16_t* written) {
        return Write(static_cast<const uint8_t*>(static_cast<const void*>(
            data)), size, written);
  }

  FilesystemStatus Read(
      char* data,
      uint16_t size,
      uint16_t* read) {
        return Read(static_cast<uint8_t*>(static_cast<void*>(
            data)), size, read);
  }

  
  uint16_t Read(uint8_t* data, uint16_t size) {
    uint16_t read;
    return Read(data, size, &read) == FS_OK ? read : 0;
  }
  uint16_t Write(const uint8_t* data, uint16_t size) {
    uint16_t written;
    return Write(data, size, &written) == FS_OK ? written : 0;
  }
  
  uint8_t eof() const { return f_eof(&f_); }
  uint8_t error() const { return f_error(&f_); }
  uint32_t tell() const { return f_tell(&f_); }
  uint32_t size() const { return f_size(&f_); }
  
 private:
  uint8_t opened_;
  FIL f_;
  
  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace avrlib

#endif   // AVRLIB_FILESYSTEM_FILE_H_
