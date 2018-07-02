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
// Driver for an array of bicolor LEDs behind N cascaded shift registers.
// The pin1 of the N * 8 - 1 LEDs are connected to the first shift register
// outputs. The pin2 of all LEDs are connected to the last output of the last
// shift register in the cascade.
// 
//  +--------+
//  | 595    |
//  |     Q0 |-----[220R]----->|---+
//  |        |                     |
//  |     Q1 |-----[220R]----->|---+
//  |        |                     |
//  |     Q2 |-----[220R]----->|---+
//  |        |                     |
//  |     Q3 |-----[220R]----->|---+
//  |        |                     |
//  |     Q4 |-----[220R]----->|---+
//  |        |                     |
//  |     Q5 |-----[220R]----->|---+
//  |        |                     |
//  |     Q6 |-----[220R]----->|---+
//  |        |                     |
//  |     Q7 |---------------------+
//  |        |
//  +--------+
//
// When Q7, is low, Q0 to Q6 are controlling the first color.
// When Q7, is high, ~Q0 to ~Q6 are controlling the second color.
//
// By toggling Q7 rapidly, the two colors can be displayed simultaneously.

#ifndef AVRLIB_DEVICES_BICOLOR_LED_ARRAY_H_
#define AVRLIB_DEVICES_BICOLOR_LED_ARRAY_H_

#include <string.h>

#include "avrlib/devices/shift_register.h"
#include "avrlib/op.h"

namespace avrlib {

template<typename Latch, typename Clock, typename Data, uint8_t num_regs = 1>
class BicolorLedArray {
 public:
  enum {
    size = num_regs * 8 - 1
  };
   
  BicolorLedArray() { }
  
  static inline void Init() {
    Register::Init();
    Clear();
  }
  
  // Intensity is in AAAABBBB format, where AAAA is the intensity for the
  // color 1, and BBBB is the intensity for the color 2.
  static inline void set_pixel(uint8_t index, uint8_t intensity) {
    buffered_pixels_[index] = intensity;
  }

  static inline void set_direct_pixel(uint8_t index, uint8_t intensity) {
    pixels_[index] = intensity;
  }
  
  static inline uint8_t pixel(uint8_t index) {
    return buffered_pixels_[index];
  }

  static inline void ShiftOutData(uint8_t v) {
    Register::ShiftOut(v);
  }
  
  static inline void Begin() {
    Register::Begin();
  }
  
  static inline void End() {
    Register::End();
  }
  
  static inline void Clear() {
    memset(buffered_pixels_, 0, size);
  }
  
  static inline void Sync() {
    memcpy(pixels_, buffered_pixels_, size);
  }
  
  static inline void ShiftOutPixels() {
    uint8_t threshold = refresh_cycle_ & 0x0f;
    uint8_t color = refresh_cycle_ & 0x20 ? 1 : 0;
    
    uint8_t byte = color;
    uint8_t num_bits = 1;
    
    for (uint8_t i = (num_regs * 8) - 2; i != 0xff ; --i) {
      byte <<= 1;
      uint8_t intensity;
      if (color) {
        intensity = U8ShiftRight4(~pixels_[i]);
      } else {
        intensity = pixels_[i] & 0x0f;
      }
      if (intensity > threshold || intensity == 0xf) {
        byte |= 1;
      }
      ++num_bits;
      if (num_bits == 8) {
        Register::ShiftOut(byte);
        num_bits = 0;
        byte = 0;
      }
    }
    ++refresh_cycle_;
  }
  
  static inline void Write() {
    Begin();
    ShiftOutPixels();
    End();
  }
  
  uint8_t* pixels() { return buffered_pixels_; }
  
 private:
  typedef ShiftRegisterOutput<Latch, Clock, Data, 8, MSB_FIRST> Register;

  static uint8_t buffered_pixels_[size];
  static uint8_t pixels_[size];
  static uint8_t refresh_cycle_;

  DISALLOW_COPY_AND_ASSIGN(BicolorLedArray);
};

template<typename Latch, typename Clock, typename Data, uint8_t num_regs>
uint8_t BicolorLedArray<Latch, Clock, Data, num_regs>::pixels_[size];

template<typename Latch, typename Clock, typename Data, uint8_t num_regs>
uint8_t BicolorLedArray<Latch, Clock, Data, num_regs>::buffered_pixels_[size];

template<typename Latch, typename Clock, typename Data, uint8_t num_regs>
uint8_t BicolorLedArray<Latch, Clock, Data, num_regs>::refresh_cycle_;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_BICOLOR_LED_ARRAY_H_

