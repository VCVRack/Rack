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
// Mini sequencer.

#ifndef PEAKS_MODULATIONS_MINI_SEQUENCER_H_
#define PEAKS_MODULATIONS_MINI_SEQUENCER_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "peaks/gate_processor.h"

namespace peaks {

const uint8_t kMaxNumSteps = 4;

class MiniSequencer {
 public:
  MiniSequencer() { }
  ~MiniSequencer() { }
  
  void Init() {
    std::fill(&steps_[0], &steps_[kMaxNumSteps], 0);
    num_steps_ = 4;
    step_ = 0;
    reset_at_next_clock_ = false;
  }
  
  inline void set_step(uint8_t index, int16_t value) {
    int32_t difference = abs(int32_t(steps_[index]) - value);
    if (difference > 0) {
      steps_[index] = value;
    }
  }
  
  inline void set_num_steps(uint8_t num_steps) {
    num_steps_ = num_steps;
  }
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_step(0, parameter[0] - 32768);
      set_step(1, parameter[1] - 32768);
      set_num_steps(2);
    } else {
      set_step(0, parameter[0] - 32768);
      set_step(1, parameter[1] - 32768);
      set_step(2, parameter[2] - 32768);
      set_step(3, parameter[3] - 32768);
      set_num_steps(4);
    }
  }
  
  void Process(const GateFlags* gate_flags, int16_t* out, size_t size) {
    while (size--) {
      GateFlags gate_flag = *gate_flags++;
      if (gate_flag & GATE_FLAG_RISING) {
        ++step_;
        if (reset_at_next_clock_) {
          reset_at_next_clock_  = false;
          step_ = 0;
        }
      }
      if (num_steps_ > 2 && gate_flag & GATE_FLAG_AUXILIARY_RISING) {
        reset_at_next_clock_ = true;
      }
      if (step_ >= num_steps_) {
        step_ = 0;
      }
      *out++ = static_cast<int32_t>(steps_[step_]) * 40960 >> 16;
    }
  }
  
 private:
  uint8_t num_steps_;
  uint8_t step_;
  int16_t steps_[kMaxNumSteps];

  bool reset_at_next_clock_;

  DISALLOW_COPY_AND_ASSIGN(MiniSequencer);
};

}  // namespace peaks

#endif  // PEAKS_MODULATIONS_MINI_SEQUENCER_H_
