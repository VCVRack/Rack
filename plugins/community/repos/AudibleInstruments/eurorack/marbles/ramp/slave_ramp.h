// Copyright 2015 Olivier Gillet.
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
// A ramp that follows a mater ramp through division/multiplication.

#ifndef MARBLES_RAMP_SLAVE_RAMP_H_
#define MARBLES_RAMP_SLAVE_RAMP_H_

#include "stmlib/stmlib.h"

#include "marbles/ramp/ramp.h"

namespace marbles {

class SlaveRamp {
 public:
  SlaveRamp() { }
  ~SlaveRamp() { }

  inline void Init() {
    phase_ = 0.0f;
    max_phase_ = kMaxRampValue;
    ratio_ = 1.0f;
    pulse_width_ = 0.0f;
    target_ = 1.0f;
    pulse_length_ = 0;
    bernoulli_ = false;
    must_complete_ = false;
  }

  // Initialize with a multiplied/divided rate compared to the master.
  inline void Init(int pattern_length, Ratio ratio, float pulse_width) {
    bernoulli_ = false;

    phase_ = 0.0f;
    max_phase_ = static_cast<float>(pattern_length) * kMaxRampValue;
    ratio_ = ratio.to_float();
    pulse_width_ = pulse_width;
    target_ = 1.0f;
    pulse_length_ = 0;
  }

  // Initialize with an adaptive slope: divide the frequency by 2 every time
  // we know we won't have to reach 1.0 at the next tick.
  inline void Init(
      bool must_complete,
      float pulse_width,
      float expected_value) {
    bernoulli_ = true;

    if (must_complete_) {
      phase_ = 0.0f;
      pulse_width_ = pulse_width;
      ratio_ = 1.0f;
      pulse_length_ = 0;
    }

    if (!must_complete) {
      ratio_ = (1.0f - phase_) * expected_value;
    } else {
      ratio_ = 1.0f - phase_;
    }
    must_complete_ = must_complete;
  }

  inline void Process(float frequency, float* phase, bool* gate) {
    float output_phase;
    if (bernoulli_) {
      phase_ += frequency * ratio_;
      output_phase = phase_;
      if (output_phase >= 1.0f) {
        output_phase = 1.0f;
      }
    } else {
      phase_ += frequency;
      if (phase_ >= max_phase_) {
        phase_ = max_phase_;
      }
      output_phase = phase_ * ratio_;
      if (output_phase > target_) {
        pulse_length_ = 0;
        target_ += 1.0f;
      }
      output_phase -= static_cast<float>(static_cast<int>(output_phase));
    }
    *phase = output_phase;
    *gate = pulse_width_ == 0.0f
        ? pulse_length_ < 32 && output_phase <= 0.5f
        : output_phase < pulse_width_;
    ++pulse_length_;
  }

 private:
  float phase_;
  float max_phase_;
  float ratio_;
  float pulse_width_;
  float target_;
  int pulse_length_;

  bool bernoulli_;
  bool must_complete_;
  
  DISALLOW_COPY_AND_ASSIGN(SlaveRamp);
};

}  // namespace marbles

#endif  // MARBLES_RAMP_SLAVE_RAMP_H_
