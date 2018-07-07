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
// Multistage envelope.

#include "peaks/modulations/multistage_envelope.h"

#include "stmlib/utils/dsp.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

void MultistageEnvelope::Init() {
  set_adsr(0, 8192, 16384, 32767);
  segment_ = num_segments_;
  phase_ = 0;
  phase_increment_ = 0;
  start_value_ = 0;
  value_ = 0;
  hard_reset_ = false;
}

int16_t MultistageEnvelope::ProcessSingleSample(uint8_t control) {
  if (control & CONTROL_GATE_RISING) {
    start_value_ = (segment_ == num_segments_ || hard_reset_)
        ? level_[0]
        : value_;
    segment_ = 0;
    phase_ = 0;
  } else if (control & CONTROL_GATE_FALLING && sustain_point_) {
    start_value_ = value_;
    segment_ = sustain_point_;
    phase_ = 0;
  } else if (phase_ < phase_increment_) {
    start_value_ = level_[segment_ + 1];
    ++segment_;
    phase_ = 0;
    if (segment_ == loop_end_) {
      segment_ = loop_start_;
    }
  }
  
  bool done = segment_ == num_segments_;
  bool sustained = sustain_point_ && segment_ == sustain_point_ &&
      control & CONTROL_GATE;

  phase_increment_ =
      sustained || done ? 0 : lut_env_increments[time_[segment_] >> 8];

  int32_t a = start_value_;
  int32_t b = level_[segment_ + 1];
  uint16_t t = Interpolate824(
      lookup_table_table[LUT_ENV_LINEAR + shape_[segment_]], phase_);
  value_ = a + ((b - a) * (t >> 1) >> 15);
  phase_ += phase_increment_;
  return value_;
}

}  // namespace peaks
