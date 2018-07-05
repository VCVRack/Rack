// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Helper functions for using a sector of flash for non-volatile storage.
//
// Because the flash memory has a limited number of erase cycles (10k to 100k)
// using flash as a permanent storage space is fine for calibration or presets,
// but should be handled with care when saving, for example, the state of a
// device - which can be updated every second!
//
// If the amount of data to save is small, it is more efficient to just append
// versions after each other without overwriting. For example, if the buffer to
// save is 16 bytes long, it is recommended to erase a page, and fill it
// progressively at each save request with the blocks of data until it is full
// and the page can be erased again. This way, only 1 erase will be needed
// every 64th call to the save function. This strategy is implemented in
// ParsimoniousLoad and ParsimoniousSave, and practically extends the life of
// the flash memory by a 40x factor in Braids.

#ifndef STMLIB_SYSTEM_SECTOR_STORAGE_H_
#define STMLIB_SYSTEM_SECTOR_STORAGE_H_

#include <stm32f4xx_conf.h>

#include <cstring>

#include "stmlib/stmlib.h"

namespace stmlib {
  
template<uint32_t> struct Sector { };

template<> struct Sector<0> { enum { start = 0x08000000 }; };
template<> struct Sector<1> { enum { start = 0x08004000 }; };
template<> struct Sector<2> { enum { start = 0x08008000 }; };
template<> struct Sector<3> { enum { start = 0x0800C000 }; };
template<> struct Sector<4> { enum { start = 0x08010000 }; };
template<> struct Sector<5> { enum { start = 0x08020000 }; };
template<> struct Sector<6> { enum { start = 0x08040000 }; };
template<> struct Sector<7> { enum { start = 0x08060000 }; };
template<> struct Sector<8> { enum { start = 0x08080000 }; };
template<> struct Sector<9> { enum { start = 0x080A0000 }; };
template<> struct Sector<10> { enum { start = 0x080C0000 }; };
template<> struct Sector<11> { enum { start = 0x080E0000 }; };
template<> struct Sector<12> { enum { start = 0x08100000 }; };

// Class for storing calibration and state data in a same sector of flash
// without having to rewrite the calibration data every time the state is
// changed.
//
// Data is stored in a RIFF-ish format, with the peculiarity that the size
// field of the header is 16-bit instead of 32-bit - the remaining 16 bits being
// used to store a naive checksum of the chunk data.
// 
// +----+-----------------+----+------------+----+------------+----+--
// |HEAD|CALIBRATION CHUNK|HEAD|STATE CHUNK1|HEAD|STATE CHUNK2|HEAD|..
// +----+-----------------+----+------------+----+------------+----+--
//
// The first chunk stores the large, "slow" data like calibration or presets.
// Whenever this data needs to be saved, the entire sector of flash is erased.
//
// The subsequent chunks store successive revision of the "fast" data like
// module state. Whenever this data changes, a new chunk is appended to the list
// until the flash sector is filled - in which case the sector is erased, and
// the calibration data + first version of state is written.
template<
    uint32_t sector_index,
    typename PersistentData,
    typename StateData>
class ChunkStorage {
 private:
  struct ChunkHeader {
    uint32_t tag;
    uint16_t size;
    uint16_t checksum;
  };

  enum {
    FLASH_STORAGE_BASE = Sector<sector_index>::start,
    FLASH_STORAGE_LAST = Sector<sector_index + 1>::start,
    FLASH_STORAGE_SIZE = FLASH_STORAGE_LAST - FLASH_STORAGE_BASE
  };

 public:
  ChunkStorage() { }
  ~ChunkStorage() { }
  // Loads the latest saved data. In case the sector is blank/corrupted,
  // reinitializes the sector and returns false.
  bool Init(PersistentData* persistent_data, StateData* state_data) {
    persistent_data_ = persistent_data;
    state_data_ = state_data;
  
    if (ReadChunk(0, persistent_data)) {
      for (next_state_chunk_index_ = 1;
           chunk_address(next_state_chunk_index_ + 1) <= FLASH_STORAGE_LAST;
           ++next_state_chunk_index_) {
         if (!ReadChunk(next_state_chunk_index_, state_data)) {
           break;
         }
      }
      if (next_state_chunk_index_ != 1) {
        return true;
      }
    }
    RewriteSector();
    return false;
  }

  void SaveState() {
    if (chunk_address(next_state_chunk_index_ + 1) > FLASH_STORAGE_LAST) {
      RewriteSector();
    } else {
      FLASH_Unlock();
      FLASH_ClearFlag(
          FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
          FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 
      WriteChunk(next_state_chunk_index_, state_data_);
      next_state_chunk_index_++;
    }
  }
  
  void SavePersistentData() {
    RewriteSector();
  }

 private:
   void RewriteSector() {
    FLASH_Unlock();
    FLASH_ClearFlag(
        FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 
    FLASH_EraseSector(sector_index * 8, VoltageRange_3);
    WriteChunk(0, persistent_data_);
    WriteChunk(1, state_data_);
    next_state_chunk_index_ = 2;
  }

  template<typename T>
  bool ReadChunk(size_t index, T* data) {
    const char* flash_ptr = (const char*)(chunk_address(index));
    ChunkHeader* h = (ChunkHeader*)(flash_ptr);
    if (h->tag == T::tag &&
        h->size == sizeof(T) &&
        Checksum(flash_ptr + sizeof(ChunkHeader), h->size) == h->checksum) {
      memcpy(data, flash_ptr + sizeof(ChunkHeader), h->size);
      return true;
    } else {
      return false;
    }
  }

  template<typename T>
  void WriteChunk(size_t index, const T* data) {
    ChunkHeader h;
    h.tag = T::tag;
    h.size = sizeof(T);
    h.checksum = Checksum(data, sizeof(T));
  
    FlashWrite(chunk_address(index), &h);
    FlashWrite(chunk_address(index) + sizeof(ChunkHeader), data);
  }

  template<typename T>
  void FlashWrite(uint32_t address, const T* data) {
    const uint32_t* words = (const uint32_t*)(data);
    size_t size = (sizeof(T) + 3) & ~0x03;
    while (size) {
      FLASH_ProgramWord(address, *words);
      address += 4;
      size -= 4;
      ++words;
    }
  }
 
  template<typename T>
  inline size_t ChunkSize() {
    return sizeof(ChunkHeader) + ((sizeof(T) + 3) & ~0x03);
  }

  uint32_t chunk_address(size_t index) {
    if (index == 0) {
      return FLASH_STORAGE_BASE;
    } else {
      return FLASH_STORAGE_BASE + ChunkSize<PersistentData>() + ChunkSize<StateData>() * (index - 1);
    }
  }

  uint16_t Checksum(const void* data, size_t size) const {
    uint16_t sum = 0;
    const char* bytes = static_cast<const char*>(data);
    while (size--) {
      sum += *bytes++;
    }
    return sum ^ 0xffff;
  }

 private:
  PersistentData* persistent_data_;
  StateData* state_data_;
  uint16_t next_state_chunk_index_;

  DISALLOW_COPY_AND_ASSIGN(ChunkStorage);
};


template<uint32_t sector_index>
class Storage {
 public:
  enum {
    FLASH_STORAGE_BASE = Sector<sector_index>::start,
    FLASH_STORAGE_LAST = Sector<sector_index + 1>::start,
    FLASH_STORAGE_SIZE = FLASH_STORAGE_LAST - FLASH_STORAGE_BASE
  };
  
  template<typename T>
  static void Save(const T& data) {
    Save((void*)(&data), sizeof(T));
  }

  static void Save(const void* data, size_t data_size) {
    FLASH_Unlock();
    FLASH_ClearFlag(
        FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 
    FLASH_EraseSector(sector_index * 8, VoltageRange_3);
    WriteBlock(FLASH_STORAGE_BASE, data, data_size);
  };
  
  template<typename T>
  static bool Load(T* data) {
    return Load((void*)(data), sizeof(T));
  }

  static bool Load(void* data, size_t data_size) {
    uint32_t base = FLASH_STORAGE_BASE;
    memcpy(data, (void*)(base), data_size);
    uint16_t checksum = (*(uint16_t*)(base + data_size));
    return checksum == Checksum(data, data_size);
  };
  
  template<typename T>
  static void ParsimoniousSave(const T& data, uint16_t* version_token) {
    return ParsimoniousSave((void*)(&data), sizeof(T), version_token);
  }
  
  static void ParsimoniousSave(
      const void* data,
      size_t data_size,
      uint16_t* version_token) {
    FLASH_Unlock();
    FLASH_ClearFlag(
        FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 

    // 2 bytes of checksum and 2 bytes of version are added to the block.
    size_t block_size = data_size + 2 + 2;
    uint32_t start = FLASH_STORAGE_BASE + block_size * *version_token;
    if (start + block_size >= FLASH_STORAGE_LAST) {
      *version_token = 0;
    }
    if (*version_token == 0) {
      start = FLASH_STORAGE_BASE;
      FLASH_EraseSector(sector_index * 8, VoltageRange_3);
    }
    WriteBlock(start, data, data_size);
    FLASH_ProgramHalfWord(start + data_size + 2, *version_token);
    *version_token = *version_token + 1;
  }
  
  template<typename T>
  static bool ParsimoniousLoad(T* data, uint16_t* version_token) {
    return ParsimoniousLoad((void*)(data), sizeof(T), version_token);
  }
  
  static bool ParsimoniousLoad(
      void* data,
      size_t data_size,
      uint16_t* version_token) {
    size_t block_size = data_size + 2 + 2;

    // Try from the end of the reserved area until we find a block with 
    // the right checksum and the right version index. 
    for (int16_t candidate_version = (FLASH_STORAGE_SIZE / block_size) - 1;
         candidate_version >= 0;
         --candidate_version) {
      uint32_t start = FLASH_STORAGE_BASE + candidate_version * block_size;
      
      memcpy(data, (void*)(start), data_size);
      uint16_t expected_checksum = Checksum(data, data_size);
      uint16_t read_checksum = (*(uint16_t*)(start + data_size));
      uint16_t version_number = (*(uint16_t*)(start + data_size + 2));
      if (read_checksum == expected_checksum &&
          version_number == candidate_version) {
        *version_token = version_number + 1;
        return true;
      }
    }
    // Memory appears to be corrupted or virgin - restart from scratch.
    *version_token = 0;
    return false;
  }
  
 private:
  static void WriteBlock(uint32_t start, const void* data, size_t data_size) {
    const uint32_t* words = (const uint32_t*)(data);
    size_t size = data_size;
    uint32_t address = start;
    while (size >= 4) {
      FLASH_ProgramWord(address, *words++);
      address += 4;
      size -= 4;
    }
    // Write checksum.
    uint16_t checksum = Checksum(data, data_size);
    FLASH_ProgramHalfWord(start + data_size, checksum);
  }
   
  static uint16_t Checksum(const void* data, uint16_t size) {
    uint16_t s = 0;
    const uint8_t* d = static_cast<const uint8_t*>(data);
    while (size--) {
      s += *d++;
    }
    return s ^ 0xffff;
  }
};

};  // namespace stmlib

#endif  // STMLIB_SYSTEM_PAGE_STORAGE_H_
