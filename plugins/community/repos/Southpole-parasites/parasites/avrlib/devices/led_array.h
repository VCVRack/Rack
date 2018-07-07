// Copyright 2012 Peter Kvitek
//
// Author: Peter Kvitek (pete@kvitek.com)
// Based on BiColorLedArray code by Olivier Gillet (ol.gillet@gmail.com)
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
// Driver for an array of LEDs behind shift registers.

#ifndef AVRLIB_DEVICES_LED_ARRAY_H_
#define AVRLIB_DEVICES_LED_ARRAY_H_

#include <string.h>

#include "avrlib/devices/shift_register.h"
#include "avrlib/op.h"

namespace avrlib {

template<typename Latch, typename Clock, typename Data, uint8_t num_regs = 1, DataOrder order = LSB_FIRST>
class LedArray {
 public:

  enum {
    size = num_regs * 8,
    max_intensity = 0x0f,
  };

  LedArray() { }
  
  static inline void Init() {
    Register::Init();
    Clear();
  }

  static inline void set_pixel(uint8_t index) {
    pixels_[index] = max_intensity;
  }

  static inline void set_pixel(uint8_t index, uint8_t intensity) {
    pixels_[index] = intensity;
  }

  static inline void clr_pixel(uint8_t index) {
    pixels_[index] = 0;
  }

  static inline uint8_t pixel(uint8_t index) {
    return pixels_[index];
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
    memset(pixels_, 0, size);
  }
  
  static inline void SetPixels(uint8_t intensity) {
    memset(pixels_, intensity, size);
  }
  
  static inline void ShiftOutPixels() {
    uint8_t threshold = refresh_cycle_ & max_intensity;
    
    uint8_t byte = 0;
    uint8_t num_bits = 0;
    
    for (uint8_t i = 0; i < size; i++) {
      byte <<= 1;
      uint8_t intensity;
      intensity = pixels_[i] & max_intensity;

      if (intensity > threshold || intensity == max_intensity) {
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
  
  uint8_t* pixels() { return pixels_; }
  
 private:
  typedef ShiftRegisterOutput<Latch, Clock, Data, 8, order> Register;

  static uint8_t pixels_[size];
  static uint8_t refresh_cycle_;

  DISALLOW_COPY_AND_ASSIGN(LedArray);
};

template<typename Latch, typename Clock, typename Data, uint8_t num_regs, DataOrder order>
uint8_t LedArray<Latch, Clock, Data, num_regs, order>::pixels_[size];

template<typename Latch, typename Clock, typename Data, uint8_t num_regs, DataOrder order>
uint8_t LedArray<Latch, Clock, Data, num_regs, order>::refresh_cycle_;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_LED_ARRAY_H_
