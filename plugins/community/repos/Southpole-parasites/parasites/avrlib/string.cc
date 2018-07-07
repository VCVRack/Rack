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

#include "avrlib/string.h"

#include <stdio.h>

namespace avrlib {

size_t strnlen(const char* string, size_t maxlen) {
  const char* end = (char*)memchr(string, '\0', maxlen);
  return end ? (size_t) (end - string) : maxlen;
}

void PadRight(char* source, uint8_t width, char character) {
  uint8_t len = strnlen(source, width);
  if (len == width) {
    return;
  }
  char* destination = source + width - 1;
  for (uint8_t i = 0; i < width; ++i) {
    if (i < len) {
      *destination-- = source[len - i - 1];
    } else {
      *destination-- = character;
    }
  }
}

void AlignRight(char* source, uint8_t width) {
  PadRight(source, width, ' ');
}

void AlignLeft(char* source, uint8_t width) {
  uint8_t len = strnlen(source, width);
  while (len < width) {
    source[len++] = ' ';
  }
}

}  // namespace avrlib
