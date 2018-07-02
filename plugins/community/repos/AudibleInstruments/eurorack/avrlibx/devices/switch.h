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
// Debouncing for:
// - A single switch.
// - An array of switches.

#ifndef AVRLIBX_DEVICES_SWITCHES_H_
#define AVRLIBX_DEVICES_SWITCHES_H_

#include "avrlibx/avrlibx.h"
#include "avrlibx/devices/shift_register.h"
#include "avrlibx/io/gpio.h"

namespace avrlibx {
  
template<typename Input, bool pulled_up = true>
class DebouncedSwitch {
 public:
  DebouncedSwitch() { }
    
  static inline void Init() {
    Input::set_direction(INPUT);
    if (pulled_up) {
      Input::set_mode(PORT_MODE_PULL_UP);
    }
    state_ = 0xff;
  }

  // To be called at a rate < 1000 Hz.
  static inline uint8_t Read() {
    state_ = (state_ << 1) | Input::value();
    return state_;
  }

  static inline uint8_t lowered() { return state_ == 0x80; }
  static inline uint8_t raised() { return state_ == 0x7f; }
  static inline uint8_t high() { return state_ == 0xff; }
  static inline uint8_t low() { return state_ == 0x00; }
  static inline uint8_t state() { return state_; }
  static inline uint8_t immediate_value() { return Input::value(); }

 private:
  static uint8_t state_;

  DISALLOW_COPY_AND_ASSIGN(DebouncedSwitch);
};

/* static */
template<typename Input, bool pulled_up>
uint8_t DebouncedSwitch<Input, pulled_up>::state_;


template<typename Load, typename Clock, typename Data, uint8_t num_inputs>
class DebouncedSwitches {
  typedef typename DataTypeForSize<num_inputs>::Type T;
  typedef ShiftRegisterInput<
      Load, Clock, Data, 8 * sizeof(T), LSB_FIRST> Register;

 public:
  DebouncedSwitches() { }

  static inline void Init() {
    Register::Init();
    memset(state_, 0xff, sizeof(state_));
  }
  
  static inline T ReadRegister() {
    return Register::Read();
  }

  static inline void Process(T value) {
    T mask = 1 << (num_inputs - 1);
    for (uint8_t i = 0; i < num_inputs; ++i) {
      state_[i] <<= 1;
      if (value & mask) {
         state_[i] |= 1;
      }
      mask >>= 1;
    }
  }
  
  static inline void Read() {
    Process(ReadRegister());
  }
  
  static inline uint8_t lowered(uint8_t index) { return state_[index] == 0x80; }
  static inline uint8_t raised(uint8_t index) { return state_[index] == 0x7f; }
  static inline uint8_t high(uint8_t index) { return state_[index] == 0xff; }
  static inline uint8_t low(uint8_t index) { return state_[index] == 0x00; }
  static inline uint8_t state(uint8_t index) { return state_[index]; }
  static inline int8_t event(uint8_t index) {
    if (lowered(index)) {
      return -1;
    } else if (raised(index)) {
      return 1;
    }
    return 0;
  }

 private:
  static uint8_t state_[num_inputs];

  DISALLOW_COPY_AND_ASSIGN(DebouncedSwitches);
};

template<typename Load, typename Clock, typename Data, uint8_t num_inputs>
uint8_t DebouncedSwitches<Load, Clock, Data, num_inputs>::state_[num_inputs];

}  // namespace avrlibx

#endif   // AVRLIBX_DEVICES_SWITCHES_H_
