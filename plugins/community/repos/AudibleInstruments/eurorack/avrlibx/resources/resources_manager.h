// Copyright 2009 Olivier Gillet.
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
// Resources manager. Support for lookup of values/strings in tables. Since
// one might not want this functionality and just use the plain program memory
// read/write function, an alias for a stripped down version without string
// table lookup is provided (SimpleResourcesManager).

#ifndef AVRLIBX_RESOURCES_MANAGER_H_
#define AVRLIBX_RESOURCES_MANAGER_H_

#include "avrlibx/avrlibx.h"

#include <string.h>
#include <avr/pgmspace.h>

namespace avrlibx {

template<
  const prog_char* const* strings,
  const prog_uint16_t* const* lookup_tables>
struct ResourcesTables {
  static inline const prog_char* const* string_table() { return strings; }
  static inline const prog_uint16_t* const* lookup_table_table() {
      return lookup_tables;
  }
};

struct NoResourcesTables {
  static inline const prog_char* const* string_table() { return NULL; }
  static inline const prog_uint16_t* const* lookup_table_table() { return NULL; }
};

template<typename ResourceId = uint8_t, typename Tables = NoResourcesTables>
class ResourcesManager {
 public:
  static inline void LoadStringResource(ResourceId resource, char* buffer,
                                        uint8_t buffer_size) {
    if (!Tables::string_table()) {
      return;
    }
    char* address = (char*)(pgm_read_word(&(Tables::string_table()[resource])));
    strncpy_P(buffer, address, buffer_size);
  }

  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(ResourceId resource, IndexType i) {
    if (!Tables::lookup_table_table()) {
      return 0;
    };
    uint16_t* address = (uint16_t*)(
    pgm_read_word(&(Tables::lookup_table_table()[resource])));
    return ResultType(pgm_read_word(address + i));
  }

  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_char* p, IndexType i) {
    return ResultType(pgm_read_byte(p + i));
  }

  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_uint8_t* p, IndexType i) {
    return ResultType(pgm_read_byte(p + i));
  }

  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_uint16_t* p, IndexType i) {
    return ResultType(pgm_read_word(p + i));
  }

  template<typename T>
  static void Load(const prog_char* p, uint8_t i, T* destination) {
    memcpy_P(destination, p + i * sizeof(T), sizeof(T));
  }
  
  template<typename T, typename U>
  static void Load(const T* p, uint8_t i, U* destination) {
    STATIC_ASSERT(sizeof(T) == sizeof(U));
    memcpy_P(destination, p + i, sizeof(T));
  }

  template<typename T>
  static void Load(const T* p, uint8_t* destination, uint16_t size) {
    memcpy_P(destination, p, size);
  }
};

typedef ResourcesManager<> SimpleResourcesManager;

}  // namespace avrlibx

#endif  // AVRLIBX_RESOURCES_MANAGER_H_
