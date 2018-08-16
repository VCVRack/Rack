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
// Provides minimal support for reading a FAT16/FAT32 file system. Limitations:
// - Read-only.
// - The API only exposes the root directory ; though there's a hacky way of
//   reading other directories through manipulation of the not-so-opaque handle.
// - Files must be represented by a 83 name when loaded. This is not a big
//   problem since this code will be used to open only one pre-determined file
//   anyway.
//
// The safe template parameter enables:
// - More error checking of arguments / call sequences.
// - Concurrent read/write/enumeration of several files/directories.

#ifndef AVRLIB_FILESYSTEM_FAT_FILE_READER_H_
#define AVRLIB_FILESYSTEM_FAT_FILE_READER_H_

#include <string.h>

#include "avrlib/avrlib.h"
#include "avrlib/time.h"

namespace avrlib {

struct Partition {
  uint8_t state;
  uint8_t start_head;
  uint16_t start_cylinder_sector;
  uint8_t type;
  uint8_t end_head;
  uint16_t end_cylinder_sector;
  uint32_t sector_offset;
  uint32_t num_sectors;
};

struct MBR {
  uint8_t code[446];
  Partition partition[4];
  uint8_t signature;
};

struct BootSector {
  uint8_t jmp_boot[3];
  char oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t sec_per_cluster;
  uint16_t reserved_sec_count;
  
  uint8_t num_fats;
  uint16_t root_entry_count;
  uint16_t total_sec;
  uint8_t media;
  uint16_t fat_size;
  uint16_t sector_per_track;
  uint16_t num_heads;
  uint32_t hidden_sec;
  uint32_t total_sector;
  
  union {
    struct {
      uint16_t drive_number;
      uint8_t ext_signature;
      uint32_t volume_id;
      char label[11];
      char fs_type[8];
      
      uint8_t padding[28];
    } fat16;
    
    struct {
      uint32_t fat_size;
      uint16_t flags;
      uint16_t version;
      uint32_t root_sector;
      uint16_t info_sector;
      uint16_t backup_sector;
      uint8_t reserved[12];
      
      uint16_t drive_number;
      uint8_t ext_signature;
      uint32_t volume_id;
      char label[11];
      char fs_type[8];
    } fat32;
  };
  
  uint8_t bootstrap_code[420];
  uint16_t signature;
};

enum DirAttribute {
  FILE_READ_ONLY = 1,
  FILE_HIDDEN = 2,
  FILE_SYSTEM = 4,
  FILE_VOLUME = 8,
  FILE_LFN = 15,
  FILE_DIRECTORY = 16,
  FILE_ARCHIVE = 32,
  FILE_ATTRIBUTES = 0x3f,
};

struct DirectoryEntry {
	char name[11];
	uint8_t	attribute;
	uint8_t reserved;
	uint8_t creation_time_tenth;
	uint32_t creation_time;
	uint16_t last_access_time;
	uint16_t first_cluster_high;
	uint32_t last_write_time;
	uint16_t first_cluster;
	uint32_t file_size;
	
  uint8_t is_volume() { return attribute & FILE_VOLUME; }
  uint8_t is_file() { return !(attribute & (FILE_VOLUME | FILE_DIRECTORY)); }
};

union Sector {
  uint8_t bytes[512];
  uint16_t words[256];
  uint32_t dwords[128];
  DirectoryEntry entries[16];
  MBR mbr;
  BootSector boot;
};

enum FatFileReaderStatus {
  FFR_OK = 0,
  FFR_ERROR_INIT,
  FFR_ERROR_READ,
  FFR_ERROR_DISK_FORMAT_ERROR,
  FFR_ERROR_NO_FAT,
  FFR_ERROR_NO_MORE_FILES,
  FFR_ERROR_BAD_FILE,
  FFR_ERROR_FILE_NOT_FOUND,
};

enum FatType {
  FFR_FAT_UNKNOWN = 0,
  FFR_FAT16 = 16,
  FFR_FAT32 = 32
};

enum HandleType {
  FFR_DIR_HANDLE = 0,
  FFR_FILE_HANDLE
};

struct FsHandle {
  HandleType type;
  
  // Position of the sector within the cluster. When this is equal to
  // sector_size_ (number of sectors per cluster), we should use the FAT to
  // follow the link to the next sector.
  uint8_t cluster_position;
  
  // Position within file (byte) or directory list (entry#).
  uint16_t cursor;
  
  // The cluster/sector to be read at the next Read/Next operation.
  uint32_t cluster;
  uint32_t sector;
  
  // The cluster read during the most recent operation.
  uint32_t current_sector;
  
  DirectoryEntry entry;
  
  uint8_t eof() { return entry.file_size == 0; }
};

template<typename Media, bool safe = false>
class FATFileReader {
 public:
  FATFileReader() { }
  
  // Init the media access layer and look for a FAT file system in the first
  // partition.
  static FatFileReaderStatus Init() {
    fat_type_ = FFR_FAT_UNKNOWN;
    if (Media::Init()) {
      return FFR_ERROR_INIT;
    }
    // Is the VBR on sector 0?
    uint32_t boot_sector = 0;
    FatFileReaderStatus status = FindBootSector(boot_sector);
    if (status == FFR_ERROR_READ || status == FFR_ERROR_DISK_FORMAT_ERROR) {
      return status;
    }
    if (status == FFR_ERROR_NO_FAT) {
      // There is a partition table. Read first partition.
      boot_sector = sector_.mbr.partition[0].sector_offset;
      status = FindBootSector(boot_sector);
      if (status != FFR_OK) {
        return status;
      }
    }
    
    // Read FS layout.
    uint32_t fat_size = sector_.boot.fat_size;
    if (fat_type_ == FFR_FAT32) {
      fat_size = sector_.boot.fat32.fat_size;
    }
    if (safe) {
      fat_size *= sector_.boot.num_fats;
    } else {
      if (sector_.boot.num_fats == 2) {
        fat_size += fat_size;
      }
    }
    fat_sector_ = boot_sector + sector_.boot.reserved_sec_count;
    cluster_size_ = sector_.boot.sec_per_cluster;
    uint32_t start = fat_sector_ + fat_size;
    
    root_dir_ = fat_type_ == FFR_FAT32 ? sector_.boot.fat32.root_sector : start;
    data_sector_ = start + (sector_.boot.root_entry_count / 16);
    
    return FFR_OK;
  }
  
  // Open the root directory and start iterating on the file list.
  static FatFileReaderStatus OpenRootDir(FsHandle* handle) {
    memset(handle, 0, sizeof(FsHandle));
    if (fat_type_ == FFR_FAT32) {
      handle->cluster = root_dir_;
      handle->sector = cluster_to_sector(root_dir_);
    } else {
      handle->sector = root_dir_;
    }
    return FFR_OK;
  }
  
  // Iterate on the next file in the opened directory.
  static FatFileReaderStatus Next(FsHandle* handle) {
    if (safe && handle->type != FFR_DIR_HANDLE) {
      return FFR_ERROR_NO_MORE_FILES;
    }
    if (safe && SyncCache(handle)) {
      return FFR_ERROR_READ;
    }
    while (1) {
      // Every 16th entry, we need to move to the next sector.
      uint8_t offset = (handle->cursor & 0x0f);
      if (offset == 0) {
        // We have reached the end of the cluster on a FAT32 system!
        if (ReadNextSector(handle)) {
          return FFR_ERROR_READ;
        }
      }
      ++handle->cursor;
      memcpy(&handle->entry, &sector_.entries[offset], sizeof(DirectoryEntry));
      // Stop if end of table is reached.
      if (handle->entry.name[0] == 0) {
        break;
      }
      // Skip volumes.
      if (handle->entry.is_volume()) {
        continue;
      }
      // Skip current directory, or deleted files
      if (handle->entry.name[0] == '.' || handle->entry.name[0] == 0xe5) {
        continue;
      }
      return FFR_OK;
    }
    return FFR_ERROR_NO_MORE_FILES;
  };
  
  // Open a file identified by a 83 name.
  static FatFileReaderStatus Open(const char* name83, FsHandle* handle) {
    OpenRootDir(handle);
    while (Next(handle) == FFR_OK) {
      if (!memcmp(name83, handle->entry.name, 11)) {
        return Open(handle);
      }
    }
    return FFR_ERROR_FILE_NOT_FOUND;
  }
  
  // Open a file identified by a directory handle.
  static FatFileReaderStatus Open(FsHandle* handle) {
    if (safe && (handle->type != FFR_DIR_HANDLE ||
        handle->entry.name[0] == 0 || 
        handle->entry.name[0] == 0xe5 ||
        (!handle->entry.is_file()))) {
      return FFR_ERROR_BAD_FILE;
    }
    LongWord c;
    c.words[0] = handle->entry.first_cluster;
    c.words[1] = handle->entry.first_cluster_high;
    uint32_t cluster = c.value;
    if (!is_valid_cluster(cluster)) {
      return FFR_ERROR_BAD_FILE;
    }
    handle->type = FFR_FILE_HANDLE;
    handle->cluster = cluster;
    handle->cluster_position = 0;
    handle->sector = cluster_to_sector(cluster);
    handle->cursor = 0;
    return FFR_OK;
  }
  
  // Read data from a file.
  static uint16_t Read(FsHandle* handle, uint16_t size, uint8_t* buffer) {
    if (safe && handle->type != FFR_FILE_HANDLE) {
      return 0;
    }
    if (safe && SyncCache(handle)) {
      return FFR_ERROR_READ;
    }
    uint16_t read = 0;
    uint32_t remaining = handle->entry.file_size;
    while (size && remaining) {
      if (handle->cursor == 0) {
        if (ReadNextSector(handle)) {
          break;
        }
      }
      uint16_t readable = 512 - handle->cursor;
      if (readable > size) {
        readable = size;
      }
      if (readable > remaining) {
        readable = remaining;
      }
      uint16_t count = readable;
      while (count) {
        *buffer++ = sector_.bytes[handle->cursor++];
        --count;
      }
      size -= readable;
      read += readable;
      remaining -= readable;
      
      if (handle->cursor == 512) {
        handle->cursor = 0;
      }
    }
    handle->entry.file_size = remaining;
    return read;
  }
  
 private:
  // Check if a cluster number is valid.
  static inline uint8_t is_valid_cluster(uint32_t cluster) {
    if (fat_type_ == FFR_FAT16) {
      return (cluster >= 2 && cluster <= 0xffef);
    } else {
      return (cluster >= 2 && cluster <= 0x0fffffef);
    }
  }
  
  // Follow the FAT linked list.
  static uint32_t NextCluster(uint32_t cluster) {
    if (cluster < 2) {
      return 0;
    }
    uint32_t fat_sector = fat_sector_;
    fat_sector += (fat_type_ == FFR_FAT16) ? (cluster >> 8) : (cluster >> 7);
    if (ReadSector(fat_sector)) {
      return 0;
    }
    uint32_t next_cluster = (fat_type_ == FFR_FAT16)
      ? sector_.words[cluster & 0xff]
      : sector_.dwords[cluster & 0x7f] & 0x0fffffff;
    return next_cluster;
  }

  // Check that the sector fetched into memory is the right one for the present
  // file read / directory iteration operation. If this is not the case, read
  // from media layer.
  static uint8_t SyncCache(FsHandle* handle) __attribute__((noinline)) {
    if (fetched_sector_ != handle->current_sector) {
      return ReadSector(handle->current_sector);
    }
    return 0;
  }
  
  // Shortcut for reading a sector into memory.
  static uint8_t ReadSector(uint32_t sector)  __attribute__((noinline)) {
    if (Media::ReadSectors(sector, 1, sector_.bytes)) {
      return 1;
    } else {
      if (safe) {
        fetched_sector_ = sector;
      }
      return 0;
    }
  }
  
  // Read the next sector for the current object (directory or file).
  // If the end of a cluster is reached, get the next cluster from the FAT.
  static FatFileReaderStatus ReadNextSector(FsHandle* handle) {
    if (handle->cluster && handle->cluster_position == cluster_size_) {
      uint32_t next_cluster = NextCluster(handle->cluster);
      if (next_cluster == 0) {
        return FFR_ERROR_READ;
      } else if (!is_valid_cluster(next_cluster)) {
        return FFR_ERROR_READ;
      }
      handle->cluster = next_cluster;
      handle->sector = cluster_to_sector(handle->cluster);
      handle->cluster_position = 0;
    }
    if (ReadSector(handle->sector)) {
      return FFR_ERROR_READ;
    }
    if (safe) {
      handle->current_sector = handle->sector;
    }
    ++handle->sector;
    ++handle->cluster_position;
    return FFR_OK;
  }
  
  // Convert a cluster index to a sector address.
  static uint32_t cluster_to_sector(uint32_t cluster) __attribute__((noinline)) {
    cluster -= 2;
    uint8_t shift = cluster_size_;
    shift >>= 1;
    while (shift) {
      shift >>= 1;
      cluster <<= 1;
    }
    return cluster + data_sector_;
  }
   
  // Look for a FAT FS at a given sector.
  static FatFileReaderStatus FindBootSector(uint32_t sector) {
    if (ReadSector(sector)) {
      return FFR_ERROR_READ; 
    }
    if (sector_.boot.signature != 0xaa55) {
      return FFR_ERROR_DISK_FORMAT_ERROR;
    }
    if (sector_.boot.fat16.fs_type[0] == 'F' && 
        sector_.boot.fat16.fs_type[1] == 'A') {
      fat_type_ = FFR_FAT16;
      return FFR_OK;
    }
    if (sector_.boot.fat32.fs_type[0] == 'F' &&
        sector_.boot.fat32.fs_type[1] == 'A') {
      fat_type_ = FFR_FAT32;
      return FFR_OK;
    }
    return FFR_ERROR_NO_FAT;
  }
  
  static uint32_t fetched_sector_;
  static Sector sector_;
  
  static FatType fat_type_;
  static uint8_t cluster_size_;

  // Root directory sector for FAT16 ; cluster for FAT32
  static uint32_t root_dir_;
  static uint32_t fat_sector_;
  static uint32_t data_sector_;
  
  DISALLOW_COPY_AND_ASSIGN(FATFileReader);
};

/* static */
template<typename M, bool s> Sector FATFileReader<M, s>::sector_;

/* static */
template<typename M, bool s> uint32_t FATFileReader<M, s>::fetched_sector_;

/* static */
template<typename M, bool s> FatType FATFileReader<M, s>::fat_type_;

/* static */
template<typename M, bool s> uint8_t FATFileReader<M, s>::cluster_size_;

/* static */
template<typename M, bool s> uint32_t FATFileReader<M, s>::fat_sector_;

/* static */
template<typename M, bool s> uint32_t FATFileReader<M, s>::root_dir_;

/* static */
template<typename M, bool s> uint32_t FATFileReader<M, s>::data_sector_;


// This is how the media access layer can be implemented.
struct DummyMediaInterface {
  static uint8_t Init() {
    return 0;
  }
  
  static uint8_t ReadSectors(uint32_t start, uint8_t num_sectors, uint8_t* data) {
    memset(data, 0, 512);
    return 0;
  }
};


}  // namespace avrlib

#endif   // AVRLIB_FILESYSTEM_FAT_FILE_READER_H_
