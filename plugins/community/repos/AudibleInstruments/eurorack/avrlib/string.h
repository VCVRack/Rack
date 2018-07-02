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
// Utility functions for string processing.

#ifndef AVRLIB_STRING_H_
#define AVRLIB_STRING_H_

#include "avrlib/base.h"
#include <string.h>

namespace avrlib {

size_t strnlen(const char* string, size_t maxlen);

void AlignRight(char* source, uint8_t width);
void AlignLeft(char* source, uint8_t width);
void PadRight(char* source, uint8_t width, char character);

template<typename T>
struct TypeInfo {
  enum {
    has_sign = 0,
    max_size = 5
  };
};

template<> struct TypeInfo<uint8_t> { enum { has_sign = 0, max_size = 3 }; };
template<> struct TypeInfo<int8_t> { enum { has_sign = 1, max_size = 4 }; };
template<> struct TypeInfo<uint16_t> { enum { has_sign = 0, max_size = 5 }; };
template<> struct TypeInfo<int16_t> { enum { has_sign = 1, max_size = 6 }; };
template<> struct TypeInfo<uint32_t> { enum { has_sign = 0, max_size = 10 }; };
template<> struct TypeInfo<int32_t> { enum { has_sign = 1, max_size = 11 }; };

static inline uint8_t NibbleToAscii(uint8_t digit) {
  return digit < 10 ? digit + 48 : digit + 87;
}

template<typename T>
void Itoa(T i, uint8_t width, char* destination) {
  unsigned char digits[TypeInfo<T>::max_size + 1];
  if (width == 0) {
    return;
  }
  if (i == 0) {
    *destination++ = '0';
    width--;
  } else {
    if (TypeInfo<T>::has_sign && i < 0) {
      *destination++ = '-';
      width--;
      i = -i;
    }
    uint8_t digit = 0;
    while (i > 0) {
      digits[digit++] = i % 10;
      i /= 10;
    }
    while (digit) {
      *destination++ = 48 + digits[--digit];
      width--;
    }
  }
  if (width) {
    *destination++ = '\0';
  }
}

// A version of Itoa that does not allocate anything on the stack, and use
// a fixed chunk of memory for reversing the digits. Caveat: if two Itoa
// operations are taking place simultaneously, the results will be mixed.
template<typename T>
void UnsafeItoa(T i, uint8_t width, char* destination) {
  static unsigned char digits[TypeInfo<T>::max_size + 1];
  if (width == 0) {
    return;
  }
  if (i == 0) {
    *destination++ = '0';
    width--;
  } else {
    if (TypeInfo<T>::has_sign && i < 0) {
      *destination++ = '-';
      width--;
      i = -i;
    }
    uint8_t digit = 0;
    while (i > 0) {
      digits[digit++] = i % 10;
      i /= 10;
    }
    while (digit) {
      *destination++ = 48 + digits[--digit];
      width--;
    }
  }
  if (width) {
    *destination++ = '\0';
  }
}

}  // namespace avrlib

#endif  // AVRLIB_STRING_H_
