// Copyright 2012 Peter Kvitek
//
// Author: Peter Kvitek (pete@kvitek.com)
// Based on RotaryEncoder and DebouncedSwitches code
// by Olivier Gillet (ol.gillet@gmail.com)
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
// Driver for an array of clickable rotary encoders.

#ifndef AVRLIB_DEVICES_ROTARY_ENCODER_ARRAY_H_
#define AVRLIB_DEVICES_ROTARY_ENCODER_ARRAY_H_

#include <string.h>

#include "avrlib/gpio.h"
#include "avrlib/time.h"

namespace avrlib {

template<
    typename Load,
    typename Clock,
    typename A,
    typename B,
    typename C,
    uint8_t size = 8>
class RotaryEncoderArray {
 public:
  RotaryEncoderArray() { }
  ~RotaryEncoderArray() { }

  static void Init() {
    Clock::set_mode(DIGITAL_OUTPUT);
    Load::set_mode(DIGITAL_OUTPUT);
    A::set_mode(DIGITAL_INPUT);
    B::set_mode(DIGITAL_INPUT);
    C::set_mode(DIGITAL_INPUT);
    Load::High();
    Clock::Low();

    memset(stateA_, 0xff, sizeof(stateA_));
    memset(stateB_, 0xff, sizeof(stateB_));
    memset(stateC_, 0xff, sizeof(stateC_));
  }

  // To catch all clicks this has to be called at rate 5KHz or higher. 
  // This executes in 35us for 8 encoders at 20MHz.
  static void Poll() {
    Load::Low();
    Load::High();

    for (uint8_t i = size; i--; ) {
      stateA_[i] = (stateA_[i] << 1) | A::value();
      stateB_[i] = (stateB_[i] << 1) | B::value();
      stateC_[i] = (stateC_[i] << 1) | C::value();
      Clock::High();
      Clock::Low();
    }
  }

  // This executes in 200ns at 20MHz
  static inline int8_t Read(uint8_t index) {
    int8_t increment = 0;
    uint8_t a = stateA_[index];
    uint8_t b = stateB_[index];
    if (a == 0x80 && ((b & 0xf0) == 0x00)) {
        increment = 1;
    } else {
      if (b == 0x80 && (a & 0xf0) == 0x00) {
        increment = -1;
      }
    }
    return increment;
  }

  static uint8_t clicked(uint8_t index) { return raised(index); }

  static inline uint8_t lowered(uint8_t index) { return stateC_[index] == 0x80; }
  static inline uint8_t raised(uint8_t index) { return stateC_[index] == 0x7f; }
  static inline uint8_t high(uint8_t index) { return stateC_[index] == 0xff; }
  static inline uint8_t low(uint8_t index) { return stateC_[index] == 0x00; }
  static inline uint8_t state(uint8_t index) { return stateC_[index]; }
  static inline int8_t event(uint8_t index) {
    if (lowered(index)) {
      return -1;
    } else if (raised(index)) {
      return 1;
    }
    return 0;
  }

 private:
  static uint8_t stateA_[size];
  static uint8_t stateB_[size];
  static uint8_t stateC_[size];

  DISALLOW_COPY_AND_ASSIGN(RotaryEncoderArray);
};

template<
    typename Load,
    typename Clock,
    typename A,
    typename B,
    typename C,
    uint8_t size>
  uint8_t RotaryEncoderArray<Load, Clock, A, B, C, size>::stateA_[size];

template<
    typename Load,
    typename Clock,
    typename A,
    typename B,
    typename C,
    uint8_t size>
  uint8_t RotaryEncoderArray<Load, Clock, A, B, C, size>::stateB_[size];

template<
    typename Load,
    typename Clock,
    typename A,
    typename B,
    typename C, uint8_t size>
  uint8_t RotaryEncoderArray<Load, Clock, A, B, C, size>::stateC_[size];

}  // namespace avrlib

#endif  // AVRLIB_DEVICES_ROTARY_ENCODER_ARRAY_H_
