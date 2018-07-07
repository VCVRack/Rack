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
// Create random repetitions of a pulse.

#include "peaks/pulse_processor/pulse_randomizer.h"

#include <algorithm>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

static const uint8_t kDownsample = 8;

void PulseRandomizer::Init() {
  repetition_probability_ = 32767;
  acceptance_probability_ = 65535;
  delay_average_ = 32767;
  delay_randomness_ = 0;

  std::fill(
      &delay_counter_[0],
      &delay_counter_[kTriggerPulseBufferSize],
      0xffff);
  
  num_pulses_ = 0;
  retrig_counter_ = 0;
}

inline uint16_t PulseRandomizer::delay() const {
  int32_t delay = delay_average_;
  delay += delay_average_ + (Random::GetSample() * delay_randomness_ >> 16);
  if (delay < 0) {
    delay = 0;
  } else if (delay > 0xffff) {
    delay = 0xffff;
  }
  return Interpolate88(lut_delay_times, delay);
}

void PulseRandomizer::FillBuffer(
    InputBuffer* input_buffer,
    OutputBuffer* output_buffer) {
  bool new_pulse = false;
  for (uint8_t i = 0; i < kBlockSize; ++i) {
    uint8_t control = input_buffer->ImmediateRead();
    new_pulse |= control & CONTROL_GATE_RISING;
  }
    
  if ((Random::GetWord() >> 16) > acceptance_probability_) {
    // Randomly ignore incoming pulses.
    new_pulse = false;
  }
  
  if (new_pulse) {
    ++num_pulses_;
  }
    
  for (uint8_t i = 0; i < kTriggerPulseBufferSize; ++i) {
    if (delay_counter_[i] == 0xffff) {
      if (new_pulse) {
        delay_counter_[i] = delay();
        new_pulse = false;
      }
    } else if (delay_counter_[i]) {
      --delay_counter_[i];
    } else {
      if ((Random::GetWord() >> 16) < repetition_probability_) {
        ++num_pulses_;
        delay_counter_[i] = delay();
      } else {
        delay_counter_[i] = 0xffff;
      }
    }
  }
    
  if (retrig_counter_) {
    --retrig_counter_;
  } else {
    if (num_pulses_) {
      retrig_counter_ = 12;
      --num_pulses_;
    }
  }
    
  uint16_t output = retrig_counter_ > 6 ? 20480 : 0;
    
  for (uint8_t i = 0; i < kBlockSize; ++i) {
    output_buffer->Overwrite(output);
  }
}

}  // namespace peaks
