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
// Controller for an array of switches read through a parallel-in > serial out
// shift register. Includes debouncing.

#ifndef AVRLIB_DEVICES_SWITCH_ARRAY_H_
#define AVRLIB_DEVICES_SWITCH_ARRAY_H_

#include "avrlib/devices/shift_register.h"
#include "avrlib/size_to_type.h"
#include "avrlib/time.h"

namespace avrlib {

struct SwitchState {
  uint8_t changed;
  uint8_t state;
  uint8_t debounced_state;
  uint32_t time;
};

struct KeyEvent {
  uint8_t id;
  uint8_t shifted;
  uint8_t hold_time;
};

template<typename Load, typename Clock, typename Data,
         uint8_t num_inputs, uint8_t shift = 0>
class SwitchArray {
  typedef typename DataTypeForSize<num_inputs>::Type T;
  typedef ShiftRegisterInput<
      Load, Clock, Data,
      8 * sizeof(T), LSB_FIRST> Register;

 public:
  SwitchArray() { }

  static void Init() {
    for (uint8_t i = 0; i < num_inputs; ++i) {
      switch_state_[i].state = 0xff;
      switch_state_[i].debounced_state = HIGH;
      switch_state_[i].time = 0;
    }
    last_event_time_ = 0;
    Register::Init();
  }

  static uint32_t last_event_time() { return last_event_time_; }
  static uint32_t idle_time() { return milliseconds() - last_event_time_; }
  static uint8_t shifted() { return switch_state_[shift].state == 0x00; }
  static void InhibitShiftRelease() { inhibit_shift_release_ = 1; }
  static const SwitchState& switch_state(uint8_t i) { return switch_state_[i]; }
  static uint8_t released() {
    for (uint8_t i = 0; i < num_inputs; ++i) {
      if (switch_state_[i].state == 0x7f &&
          (i != shift || !inhibit_shift_release_)) {
        return 1;
      }
    }
    return 0;
  }

  static void Touch() {
    last_event_time_ = milliseconds();
  }

  static inline KeyEvent key_event() {
    KeyEvent e;
    e.id = num_inputs;
    for (uint8_t i = 0; i < num_inputs; ++i) {
      if (switch_state_[i].state == 0x7f) {
        if (i == shift && inhibit_shift_release_) {
          inhibit_shift_release_ = 0;
        } else {
          e.id = i;
          e.shifted = shifted();
          e.hold_time = static_cast<uint16_t>(
              last_event_time_ - switch_state_[i].time) >> 8;
          if (e.shifted) {
            inhibit_shift_release_ = 1;
          }
        }
      }
    }
    return e;
  }
  
  static uint8_t Read() {
    T value = Register::Read();
    uint32_t now = milliseconds();
    T mask = 1 << (num_inputs - 1);
    for (uint8_t i = 0; i < num_inputs; ++i) {
      switch_state_[i].state <<= 1;
      if (value & mask) {
         switch_state_[i].state |= 1;
      }
      if (switch_state_[i].state == 0x80) {
        last_event_time_ = now;
        switch_state_[i].debounced_state = LOW;
        switch_state_[i].time = now;
        inhibit_shift_release_ = 0;
      } else if (switch_state_[i].state == 0x7f) {
        last_event_time_ = now;
        switch_state_[i].debounced_state = HIGH;
      }
      mask >>= 1;
    }
  }

 private:
  static uint32_t last_event_time_;
  static SwitchState switch_state_[num_inputs];
  static uint8_t inhibit_shift_release_;

  DISALLOW_COPY_AND_ASSIGN(SwitchArray);
};

template<typename Load, typename Clock, typename Data, uint8_t num_inputs,
         uint8_t shift>
SwitchState SwitchArray<Load, Clock, Data, num_inputs,
                        shift>::switch_state_[num_inputs];

template<typename Load, typename Clock, typename Data, uint8_t num_inputs,
         uint8_t shift>
uint32_t SwitchArray<Load, Clock, Data, num_inputs, shift>::last_event_time_;

template<typename Load, typename Clock, typename Data, uint8_t num_inputs,
         uint8_t shift>
uint8_t SwitchArray<Load, Clock, Data, num_inputs,
                    shift>::inhibit_shift_release_;


}  // namespace avrlib

#endif   // AVRLIB_DEVICES_SWITCH_ARRAY_H_
