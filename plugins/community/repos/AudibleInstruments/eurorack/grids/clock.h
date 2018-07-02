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
// Global clock. This works as a 31-bit phase increment counter. To implement
// swing, the value at which the counter wraps is (1 << 31) times a swing
// factor.

#ifndef GRIDS_CLOCK_H_
#define GRIDS_CLOCK_H_

#include "avrlib/base.h"

#include "grids/pattern_generator.h"

namespace grids {

class Clock {
 public:
  Clock() { }
  ~Clock() { }
   
  static inline void Init() {
    Update(120, CLOCK_RESOLUTION_24_PPQN);
    locked_ = false;
  }
  static void Update(uint16_t bpm, ClockResolution resolution);
  
  static inline void Reset() {
    phase_ = 0;
  }
  
  static inline void Tick() { phase_ += phase_increment_; }
  static inline void Wrap(int8_t amount) {
    LongWord* w = (LongWord*)(&phase_);
    if (amount == 0) {
      w->bytes[3] &= 0x7f;
      falling_edge_ = 0x40;
    } else {
      if (w->bytes[3] >= 128 + amount) {
        w->bytes[3] = 0;
      }
      falling_edge_ = (128 + amount) >> 1;
    }
  }

  static inline bool raising_edge() { return phase_ < phase_increment_; }
  static inline bool past_falling_edge() {
    LongWord w;
    w.value = phase_;
    return w.bytes[3] >= falling_edge_;
  }
  
  static inline void Lock() { locked_ = true; }
  static inline void Unlock() { locked_ = false; }
  static inline bool locked() { return locked_; }
  static inline uint16_t bpm() { return bpm_; }
  
 private:
  static bool locked_;
  static uint16_t bpm_;
  static uint32_t phase_;
  static uint32_t phase_increment_;
  static uint8_t falling_edge_;
  
  DISALLOW_COPY_AND_ASSIGN(Clock);
};

extern Clock clock;

}  // namespace grids

#endif // GRIDS_CLOCK_H_
