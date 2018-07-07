// Copyright 2013 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Internal clock.

#ifndef YARNS_INTERNAL_CLOCK_H_
#define YARNS_INTERNAL_CLOCK_H_

#include "stmlib/stmlib.h"

namespace yarns {

class InternalClock {
 public:

  InternalClock() {
    phase_ = 0;
    phase_increment_ = 0;
  }

  ~InternalClock() { }
  
  inline void Start(uint32_t tempo, uint8_t swing) {
    phase_ = 0;
    swing_step_ = 11;
    set_tempo(tempo);
    set_swing(swing);
  }
  
  inline void set_tempo(uint32_t tempo) {
    phase_increment_ = 178957UL * tempo / 10;  // For 48kHz
    // phase_increment_ = 128849UL * tempo / 6;  // For 40kHz
  }
  
  inline void set_swing(uint32_t swing) {
    swing_amount_ = swing * (0x80000000 / 3 / 100);
  }
  
  inline bool Process() {
    uint32_t half_cycle = 0x80000000;
    if (swing_step_ < 6) {
      half_cycle += swing_amount_;
    } else {
      half_cycle -= swing_amount_;
    }
    
    bool tick = false;
    if (phase_ >= half_cycle) {
      tick = true;
      phase_ -= half_cycle;
      ++swing_step_;
      if (swing_step_ >= 12) {
        swing_step_ = 0;
      }
    }
    phase_ += phase_increment_;
    return tick;
  }
  
 private:
  uint32_t phase_;
  uint32_t phase_increment_;
  uint32_t swing_amount_;
  uint8_t swing_step_;
   
  DISALLOW_COPY_AND_ASSIGN(InternalClock);
};

}  // namespace yarns

#endif  // YARNS_INTERNAL_CLOCK_H_
