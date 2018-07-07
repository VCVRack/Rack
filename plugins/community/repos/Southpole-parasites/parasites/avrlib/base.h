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
// Base header.

#ifndef AVRLIB_BASE_H_
#define AVRLIB_BASE_H_

#include <inttypes.h>

#ifndef NULL
#define NULL 0
#endif

typedef union {
  uint16_t value;
  uint8_t bytes[2];
} Word;

typedef union {
  uint32_t value;
  uint16_t words[2];
  uint8_t bytes[4];
} LongWord;

struct uint24_t {
  uint16_t integral;
  uint8_t fractional;
};

struct uint24c_t {
  uint8_t carry;
  uint16_t integral;
  uint8_t fractional;
};

template<uint32_t a, uint32_t b, uint32_t c, uint32_t d>
struct FourCC {
  static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
};

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

template<bool b>
inline void StaticAssertImplementation() {
	char static_assert_size_mismatch[b] = { 0 };
}
 
#define STATIC_ASSERT(expression) StaticAssertImplementation<(expression)>()

#endif  // AVRLIB_BASE_H_
