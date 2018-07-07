// Copyright 2012 Olivier Gillet.
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
// Helper functions for using the last pages of flash for non-volatile storage.
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

#ifndef STMLIB_SYSTEM_PAGE_STORAGE_H_
#define STMLIB_SYSTEM_PAGE_STORAGE_H_

#ifdef STM32F37X
  #include <stm32f37x_conf.h>
#else
  #include <stm32f10x_conf.h>
#endif

#include <cstring>

#include "stmlib/stmlib.h"
#include "stmlib/system/flash_programming.h"

namespace stmlib {

template<uint32_t last_address = 0x8020000, uint16_t num_pages = 1>
class Storage {
 public:
  enum {
    FLASH_STORAGE_BASE = last_address - num_pages * PAGE_SIZE
  };
  
  template<typename T>
  static void Save(const T& data) {
    Save(data, 0);
  }
  
  template<typename T>
  static void Save(const T& data, uint8_t page_index) {
    Save((void*)(&data), sizeof(T), page_index);
  }

  static void Save(const void* data, size_t data_size, uint8_t page_index) {
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_STORAGE_BASE + page_index * PAGE_SIZE);
    WriteBlock(FLASH_STORAGE_BASE + page_index * PAGE_SIZE, data, data_size);
  };
  
  template<typename T>
  static bool Load(T* data) {
    return Load(data, 0);
  }
  
  template<typename T>
  static bool Load(T* data, uint8_t page_index) {
    return Load((void*)(data), sizeof(T), page_index);
  }

  static bool Load(void* data, size_t data_size, uint8_t page_index) {
    uint32_t base = FLASH_STORAGE_BASE + page_index * PAGE_SIZE;
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
    bool wrapped_around = false;
    
    // 2 bytes of checksum and 2 bytes of version are added to the block.
    size_t block_size = data_size + 2 + 2;
    uint32_t start = FLASH_STORAGE_BASE + block_size * *version_token;
    if (start + block_size >= last_address) {
      // Erase all pages and restart the versioning from scratch.
      *version_token = 0;
      start = FLASH_STORAGE_BASE;
      wrapped_around = true;
    }
    FLASH_Unlock();
    
    if (wrapped_around) {
      for (size_t i = 0; i < num_pages; ++i) {
        FLASH_ErasePage(FLASH_STORAGE_BASE + i * PAGE_SIZE);
      }
    } else {
      // If we will write into a new page, erase it.
      uint32_t previous_page = start - 1;
      previous_page -= previous_page % PAGE_SIZE;
      uint32_t this_page = start + block_size;
      this_page -= this_page % PAGE_SIZE;
      if (this_page != previous_page) {
        FLASH_ErasePage(this_page);
      }
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
    for (int16_t candidate_version = (num_pages * PAGE_SIZE / block_size) - 1;
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
    return s;
  }
};

};  // namespace stmlib

#endif  // STMLIB_SYSTEM_PAGE_STORAGE_H_
