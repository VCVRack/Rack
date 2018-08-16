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
// Controller for an array of digital outputs behind a shift register. This
// class maintains an internal array reflecting the status of each output. A
// call to Output() writes the content of the array to a shift register, if the
// content of the array has changed.
//
// The num_intensity_level template parameter allows basic PWM on the output
// of the array. More specifically, each call to Output() will write a 0 or 1
// depending on the "analog" level. This can be used for example to adjust
// the brightness of a LED. More precisely, this is achieved in a PWM-like way
// (we cycle N times through the array and at time i, we power only the outputs
// with level above i/N).

#ifndef AVRLIB_DEVICES_OUTPUT_ARRAY_H_
#define AVRLIB_DEVICES_OUTPUT_ARRAY_H_

#include <string.h>

#include "avrlib/devices/shift_register.h"
#include "avrlib/size_to_type.h"

namespace avrlib {

// A generic, not very efficient implementation, especially for small bit
// depths.
template<typename Latch, typename Clock, typename Data,
         uint8_t size = 16, uint8_t bit_depth = 1,
         DataOrder order = LSB_FIRST, bool safe = true>
class OutputArray {
 public:
  OutputArray() { }
  typedef ShiftRegisterOutput<Latch, Clock, Data, size, order> Register;
  typedef typename DataTypeForSize<bit_depth>::Type Value;
  typedef typename DataTypeForSize<size>::Type Index;
  static inline void Init() {
    if (safe) {
      memset(values_, 0, sizeof(values_));
      cycle_ = 0;
    }
    Register::Init();
  }
  static inline void set_value(Index output_index, Value intensity) {
    values_[output_index] = intensity;
  }
  static inline Value value(Index output_index) {
    return values_[output_index];
  }
  static inline void ShiftOutByte(uint8_t v) {
    Register::ShiftOut(v);
  }
  static inline void Begin() {
    Register::Begin();
  }
  static inline void End() {
    Register::End();
  }
  static inline void Clear() { memset(values_, 0, sizeof(values_)); }
  
  static inline void ShiftOut() {
    Index c = 0;
    Index bit = 1;
    for (Index i = 0; i < size; ++i) {
      if (values_[i] > cycle_ || values_[i] == ((1 << bit_depth) - 1)) {
        c |= bit;
      }
      bit <<= 1;
    }
    cycle_ = (cycle_ + 1) & ((1 << bit_depth) - 1);
    Register::ShiftOut(c);
  }
  static inline void Write() {
    Begin();
    ShiftOut();
    End();
  }
 private:
  static Value values_[size];
  static Value cycle_;

  DISALLOW_COPY_AND_ASSIGN(OutputArray);
};

template<typename Latch, typename Clock, typename Data,
         uint8_t size, uint8_t bit_depth, DataOrder order, bool safe>
typename OutputArray<Latch, Clock, Data, size, bit_depth, order, safe>::Value
OutputArray<Latch, Clock, Data, size, bit_depth, order, safe>::values_[size];

template<typename Latch, typename Clock, typename Data,
         uint8_t size, uint8_t bit_depth, DataOrder order, bool safe>
typename OutputArray<Latch, Clock, Data, size, bit_depth, order, safe>::Value
OutputArray<Latch, Clock, Data, size, bit_depth, order, safe>::cycle_;


// A specialization with only two states. In this case, we can write data to the
// shift register only when a bit has changed in the array - there's no pseudo
// PWM brightness modulation.
template<typename Latch, typename Clock, typename Data,
         uint8_t size, DataOrder order, bool safe>
class OutputArray<Latch, Clock, Data, size, 1, order, safe> {
  typedef ShiftRegisterOutput<Latch, Clock, Data, size, order> Register;
 public:
  typedef typename DataTypeForSize<size>::Type T;
  OutputArray() { }
  static inline void Init() {
    if (safe) {
      last_bits_ = 1;
      bits_ = 0;
    }
    Register::Init();
  }
  static inline void set_value(uint8_t output_index, uint8_t intensity) {
    T mask = T(1) << output_index;
    if (intensity) {
      bits_ |= mask;
    } else {
      bits_ &= ~mask;
    }
  }
  static inline void Clear() { bits_ = 0; }
  static inline uint8_t value(uint8_t output_index) {
    T mask = T(1) << output_index;
    return T(bits_ & mask) ? 1 : 0;
  }
  static inline void ShiftOutByte(uint8_t v) {
    Register::ShiftOut(v);
  }
  static inline void Begin() {
    Register::Begin();
  }
  static inline void End() {
    Register::End();
  }
  static inline void ShiftOut() {
    if (bits_ == last_bits_) {
      return;
    }
    Register::ShiftOut(bits_);
    last_bits_ = bits_;
  }
  static inline void Write() {
    Begin();
    ShiftOut();
    End();
  }
 private:
  static T bits_;
  static T last_bits_;

  DISALLOW_COPY_AND_ASSIGN(OutputArray);
};

template<typename Latch, typename Clock, typename Data,
         uint8_t size, DataOrder order, bool safe>
typename OutputArray<Latch, Clock, Data, size, 1, order, safe>::T
OutputArray<Latch, Clock, Data, size, 1, order, safe>::bits_;

template<typename Latch, typename Clock, typename Data,
         uint8_t size, DataOrder order, bool safe>
typename OutputArray<Latch, Clock, Data, size, 1, order, safe>::T
OutputArray<Latch, Clock, Data, size, 1, order, safe>::last_bits_;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_OUTPUT_ARRAY_H_
