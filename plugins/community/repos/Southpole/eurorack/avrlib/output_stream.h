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
// Output stream. Wraps any module implementing the basic Output interface
// (in fact, just a Write method), and provide string and integer formatting
// using the << stream operator.

#ifndef AVRLIB_OUTPUT_STREAM_H_
#define AVRLIB_OUTPUT_STREAM_H_

#include "avrlib/string.h"

using avrlib::Itoa;
using avrlib::TypeInfo;

namespace avrlib {

enum EndOfLine {
  endl = 0
};

template<typename Output>
struct OutputStream {
  static void Print(const char* string) {
    while (*string) {
      Output::Write(*string++);
    }
  }
  static void Print(uint8_t byte) {
    Output::Write(byte);
  }
  static void Print(char byte) {
    Output::Write(byte);
  }
  static void Print(uint16_t value) {
    char buffer[TypeInfo<uint16_t>::max_size + 1];
    Itoa(value, TypeInfo<uint16_t>::max_size + 1, buffer);
    Print(buffer);
  }
  static void Print(int16_t value) {
    char buffer[TypeInfo<int16_t>::max_size + 1];
    Itoa(value, TypeInfo<int16_t>::max_size + 1, buffer);
    Print(buffer);
  }
  static void Print(uint32_t value) {
    char buffer[TypeInfo<uint32_t>::max_size + 1];
    Itoa(value, TypeInfo<uint32_t>::max_size + 1, buffer);
    Print(buffer);
  }
  static void Print(EndOfLine e) {
    Print('\r');
    Print('\n');
  }
  template<class T>
  inline OutputStream<Output>& operator<<(const T value) {
    Print(value);
    return *this;
  }
};

}  // namespace avrlib

#endif   // AVRLIB_OUTPUT_STREAM_H_
