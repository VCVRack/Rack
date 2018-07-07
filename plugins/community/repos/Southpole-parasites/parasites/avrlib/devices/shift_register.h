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
// Driver for a 8-bits shift register.

#ifndef AVRLIB_DEVICES_SHIFT_REGISTER_H_
#define AVRLIB_DEVICES_SHIFT_REGISTER_H_

#include "avrlib/gpio.h"
#include "avrlib/size_to_type.h"
#include "avrlib/time.h"

namespace avrlib {

template<typename Latch, typename Clock, typename Data>
struct BaseShiftRegisterOutput {
  BaseShiftRegisterOutput() { }
  static void Init() {
    Clock::set_mode(DIGITAL_OUTPUT);
    Latch::set_mode(DIGITAL_OUTPUT);
    Data::set_mode(DIGITAL_OUTPUT);
    Latch::High();
  }
};

template<typename Latch, typename Clock, typename Data,
         uint8_t size = 8, DataOrder order = LSB_FIRST>
struct ShiftRegisterOutput : public BaseShiftRegisterOutput<Latch, Clock, Data> {
};

template<typename Latch, typename Clock, typename Data,
         uint8_t size>
struct ShiftRegisterOutput<Latch, Clock, Data, size, LSB_FIRST>
  : public BaseShiftRegisterOutput<Latch, Clock, Data> {
  ShiftRegisterOutput() { }
  static void ShiftOut(typename DataTypeForSize<size>::Type data) {
    Data::Low();
    for (uint8_t i = size; i > 0; --i) {
      Clock::Low();
      Data::set_value(data & 1);
      data >>= 1;
      Clock::High();
    }
    Clock::Low();
  }
  static void Begin() {
    Latch::Low();
  }
  static void End() {
    Latch::High();
  }
  static void Write(typename DataTypeForSize<size>::Type data) {
    Begin();
    ShiftOut(data);
    End();
  }
};

template<typename Latch, typename Clock, typename Data, uint8_t size>
struct ShiftRegisterOutput<Latch, Clock, Data, size, MSB_FIRST>
  : public BaseShiftRegisterOutput<Latch, Clock, Data> {
  ShiftRegisterOutput() { }
  typedef typename DataTypeForSize<size>::Type T;
  static void ShiftOut(T data) {
    Data::Low();
    T mask = (T(1) << (size - 1));
    for (uint8_t i = size; i > 0; --i) {
      Clock::Low();
      if (data & mask) {
        Data::High();
      } else {
        Data::Low();
      }
      mask >>= 1;
      Clock::High();
    }
    Clock::Low();
  }
  static void Begin() {
    Latch::Low();
  }
  static void End() {
    Latch::High();
  }
  static void Write(typename DataTypeForSize<size>::Type data) {
    Begin();
    ShiftOut(data);
    End();
  }
};

template<typename Load, typename Clock, typename Data>
struct BaseShiftRegisterInput {
  BaseShiftRegisterInput() { }
  static void Init() {
    Clock::set_mode(DIGITAL_OUTPUT);
    Load::set_mode(DIGITAL_OUTPUT);
    Data::set_mode(DIGITAL_INPUT);
    Load::High();
    Clock::Low();
  }
};

template<typename Load, typename Clock, typename Data,
         uint8_t size = 8, DataOrder order = LSB_FIRST>
struct ShiftRegisterInput : public BaseShiftRegisterInput<Load, Clock, Data> {
};

template<typename Load, typename Clock, typename Data,
         uint8_t size>
struct ShiftRegisterInput<Load, Clock, Data, size, LSB_FIRST>
  : public BaseShiftRegisterInput<Load, Clock, Data> {
  ShiftRegisterInput() { }
  typedef typename DataTypeForSize<size>::Type T;
  static T Read() {
    T data = 0;
    // Strobe load pin.
    Load::Low();
    Load::High();
    for (uint8_t i = size; i > 0; --i) {
      data <<= 1;
      data |= Data::value();
      Clock::High();
      Clock::Low();
    }
    return data;
  }
};

template<typename Load, typename Clock, typename Data, uint8_t size>
struct ShiftRegisterInput<Load, Clock, Data, size, MSB_FIRST>
  : public BaseShiftRegisterInput<Load, Clock, Data> {
  ShiftRegisterInput() { }
  typedef typename DataTypeForSize<size>::Type T;
  static T Read() {
    T mask = 1;
    T data = 0;
    // Strobe load pin.
    Load::Low();
    Load::High();
    for (uint8_t i = size; i > 0; --i) {
      if (Data::value()) {
        data |= mask;
      }
      Clock::High();
      mask <<= 1;
      Clock::Low();
    }
    return data;
  }
};

}  // namespace avrlib

#endif  // AVRLIB_DEVICES_SHIFT_REGISTER_H_
