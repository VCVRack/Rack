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
// Trigger to gate converter with pre-delay, duration, and repetitions.

#include "peaks/pulse_processor/pulse_shaper.h"

#include <algorithm>

#include "stmlib/utils/dsp.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

void PulseShaper::Init() {
  initial_delay_ = 0;
  duration_ = 0;
  delay_ = 0;
  num_repetitions_ = 0;

  Pulse p;
  p.initial_delay_counter = 0;
  p.duration_counter = 0;
  p.delay_counter = 0;
  p.repetition_counter = 0;
  
  std::fill(&pulse_buffer_[0], &pulse_buffer_[kPulseBufferSize], p);
  
  previous_num_pulses_ = 0;
  retrig_counter_ = 0;
}

inline uint16_t PulseShaper::delay() const {
  return Interpolate88(lut_delay_times, delay_) - 1;
}

inline uint16_t PulseShaper::duration() const {
  return Interpolate88(lut_delay_times, duration_);
}

inline uint16_t PulseShaper::initial_delay() const {
  return Interpolate88(lut_delay_times, initial_delay_);
}

void PulseShaper::FillBuffer(
    InputBuffer* input_buffer,
    OutputBuffer* output_buffer) {
  bool new_pulse = false;
  for (uint8_t i = 0; i < kBlockSize; ++i) {
    uint8_t control = input_buffer->ImmediateRead();
    new_pulse |= control & CONTROL_GATE_RISING;
  }
    
  uint8_t num_pulses = 0;
  for (uint8_t i = 0; i < kPulseBufferSize; ++i) {
    Pulse& p = pulse_buffer_[i];
    if (p.repetition_counter) {
      // Handle the case when the duration of the pulse is larger than the
      // delay time. In this case, set the duration to just a sample below
      // the delay time.
      if (p.delay_counter < p.duration_counter && p.repetition_counter > 1) {
        p.duration_counter = p.delay_counter;
      }
      
      if (p.initial_delay_counter == 0) {
        if (p.duration_counter) {
          // ON
          --p.duration_counter;
          ++num_pulses;
        }
        if (p.delay_counter) {
          --p.delay_counter;
        } else {
          // Retrigger
          --p.repetition_counter;
          p.duration_counter = duration();
          p.delay_counter = delay();
        }
      } else {
        // Still in pre-delay phase...
        --p.initial_delay_counter;
      }
    } else {
      if (new_pulse) {
        p.repetition_counter = num_repetitions_ + 1;
        p.initial_delay_counter = initial_delay();
        p.duration_counter = duration();
        p.delay_counter = delay();
        new_pulse = false;
        num_pulses += p.initial_delay_counter ? 0 : 1;
      }
    }
  }
    
  // The output is already high, but a new pulse is arriving. Create
  // a short dip in the output to retrigger.
  if (previous_num_pulses_ && num_pulses > previous_num_pulses_) {
    retrig_counter_ = 6;
  }
  previous_num_pulses_ = num_pulses;

  if (retrig_counter_) {
    --retrig_counter_;
  }
  uint16_t output = num_pulses > 0 && !retrig_counter_ ? 20480 : 0;
    
  for (uint8_t i = 0; i < kBlockSize; ++i) {
    output_buffer->Overwrite(output);
  }
}

}  // namespace peaks
