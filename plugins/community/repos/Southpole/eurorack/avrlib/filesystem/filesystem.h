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

#ifndef AVRLIB_FILESYSTEM_FILE_SYSTEM_H_
#define AVRLIB_FILESYSTEM_FILE_SYSTEM_H_

#include <string.h>

#include "avrlib/avrlib.h"

#include "avrlib/third_party/ff/ff.h"
#include "avrlib/third_party/ff/mmc.h"

namespace avrlib {

enum FilesystemStatus {
  FS_OK = 0,
  FS_DISK_ERROR,
  FS_EXCEPTION,
  FS_DRIVE_NOT_READY,
  FS_FILE_NOT_FOUND,
  FS_PATH_NOT_FOUND,
  FS_INVALID_NAME,
  FS_ACCESS_DENIED,
  FS_FILE_EXISTS,
  FS_INVALID_OBJECT,
  FS_WRITE_PROTECTED,
  FS_INVALID_DRIVE,
  FS_VOLUME_NOT_INITIALIZED,
  FS_NO_FAT_VOLUME,
  FS_FORMAT_FAILED,
  FS_TIMEOUT,
  FS_LOCKED,
  FS_NOT_ENOUGH_MEMORY,
  FS_TOO_MANY_FILES,
  FS_INVALID_PARAMETER,
  FS_NOT_OPENED,
  FS_BAD_FILE_FORMAT,
  FS_COPY_ERROR
};

enum FileAttribute {
  FS_ATTRIBUTE_READ_ONLY = 1,
  FS_ATTRIBUTE_HIDDEN = 2,
  FS_ATTRIBUTE_SYSTEM = 4,
  FS_ATTRIBUTE_VOLUME = 8,
  FS_ATTRIBUTE_LFN = 15,
  FS_ATTRIBUTE_DIRECTORY = 16,
  FS_ATTRIBUTE_ARCHIVE = 32,
  FS_ATTRIBUTE_ATTRIBUTES = 0x3f,
};

struct FileInfo {
  inline uint32_t size() const {
    return file_info.fsize;
  }
  
  inline uint16_t modification_date() const {
    return file_info.fdate;
  }
  
  inline uint16_t modification_time() const {
    return file_info.ftime;
  }

  inline uint8_t attributes() const {
    return file_info.fattrib;
  }
  
  inline uint8_t is_read_only() const {
    return file_info.fattrib & FS_ATTRIBUTE_READ_ONLY;
  }

  inline uint8_t is_hidden() const {
    return file_info.fattrib & FS_ATTRIBUTE_HIDDEN;
  }

  inline uint8_t is_system() const {
    return file_info.fattrib & FS_ATTRIBUTE_SYSTEM;
  }

  inline uint8_t is_volume() const {
    return file_info.fattrib & FS_ATTRIBUTE_VOLUME;
  }

  inline uint8_t is_directory() const {
    return file_info.fattrib & FS_ATTRIBUTE_DIRECTORY;
  }

  inline uint8_t is_archive() const {
    return file_info.fattrib & FS_ATTRIBUTE_ARCHIVE;
  }
  
  inline const char* name() const {
    return file_info.fname;
  }
  
  FILINFO file_info;
};

class Filesystem {
 public:
  Filesystem() { }
  
  static FilesystemStatus Init();
  static FilesystemStatus Init(uint16_t timeout_ms);
  
  static FilesystemStatus Unlink(const char* file_name);
  static FilesystemStatus Mkdir(const char* dir_name);
  static FilesystemStatus Mkdirs(char* path);
  static FilesystemStatus Chmod(
      const char* file_name,
      uint8_t value,
      uint8_t mask);
  static FilesystemStatus Rename(const char* old_name, const char* new_name);
  static FilesystemStatus FileStatus(const char* file_name, FileInfo* info);
  static FilesystemStatus Utime(
      const char* file_name,
      uint16_t date,
      uint16_t time);
  
  static FilesystemStatus Mkfs();
  
  static uint32_t GetFreeSpace();
  static uint16_t GetType() {
    uint8_t card_type;
    disk_ioctl(0, MMC_GET_TYPE, &card_type);
    return fs_.fs_type | (card_type << 8);
  }
  
  static inline void Tick() {
    disk_timerproc();
  }
  
  static uint8_t* buffer() { return fs_.win; }
  
 private:
  static FATFS fs_;
  
  DISALLOW_COPY_AND_ASSIGN(Filesystem);
};

}  // namespace avrlib

#endif   // AVRLIB_FILESYSTEM_FILE_SYSTEM_H_
