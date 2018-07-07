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

#include "avrlib/filesystem/filesystem.h"
#include "avrlib/time.h"

namespace avrlib {

/* static */
FATFS Filesystem::fs_;

/* static */
FilesystemStatus Filesystem::Init() {
  f_mount(0, &fs_);
  return (disk_initialize(0) & STA_NOINIT) ? FS_DISK_ERROR : FS_OK;
}

/* static */
FilesystemStatus Filesystem::Init(uint16_t timeout_ms) {
  f_mount(0, &fs_);
  for (uint32_t t = milliseconds() + timeout_ms; milliseconds() < t; ) {
    if (!(disk_initialize(0) & STA_NOINIT)) {
      return FS_OK;
    }
  }
  return FS_DISK_ERROR;
}

/* static */
FilesystemStatus Filesystem::Unlink(const char* file_name) {
  return static_cast<FilesystemStatus>(f_unlink(file_name));
}

/* static */
FilesystemStatus Filesystem::Mkdir(const char* dir_name) {
  return static_cast<FilesystemStatus>(f_mkdir(dir_name));
}

/* static */
FilesystemStatus Filesystem::Mkdirs(char* path) {
  for (char* p = path + 1; *p; ++p) {
    // For each path prefix, attempt to create the path.
    if (*p == '/') {
      *p = '\0';
      FilesystemStatus status = Mkdir(path);
      *p = '/';
      if (status != FS_OK && status != FS_FILE_EXISTS) {
        return status;
      }
    }
  }
  return FS_OK;
}

/* static */
FilesystemStatus Filesystem::Chmod(
    const char* file_name,
    uint8_t value,
    uint8_t mask) {
  return static_cast<FilesystemStatus>(f_chmod(file_name, value, mask));
}

/* static */
FilesystemStatus Filesystem::Rename(
    const char* old_name,
    const char* new_name) {
  return static_cast<FilesystemStatus>(f_rename(old_name, new_name));
}

/* static */
FilesystemStatus Filesystem::Mkfs() {
  return static_cast<FilesystemStatus>(f_mkfs(0, 0, 0));
}
  
/* static */
uint32_t Filesystem::GetFreeSpace() {
  FATFS* p;
  DWORD free_clusters = 0;
  f_getfree("/", &free_clusters, &p);
  return free_clusters * p->csize * 512;  // 0 if error.
}

/* static */
FilesystemStatus Filesystem::FileStatus(const char* file_name, FileInfo* info) {
  return static_cast<FilesystemStatus>(f_stat(file_name, &info->file_info));
}

/* static */
FilesystemStatus Filesystem::Utime(
    const char* file_name,
    uint16_t date,
    uint16_t time) {
  FILINFO f;
  f.fdate = date;
  f.ftime = time;
  return static_cast<FilesystemStatus>(f_utime(file_name, &f));
}

}  // namespace avrlib
