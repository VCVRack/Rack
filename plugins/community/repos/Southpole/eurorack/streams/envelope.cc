// Copyright 2014 Olivier Gillet.
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
// Simple AD envelope - adapted from Peaks' multistage envelope.

#include "streams/envelope.h"

#include "stmlib/utils/dsp.h"

#include "streams/resources.h"

#include "streams/gain.h"

#include <algorithm>

namespace streams {

using namespace std;
using namespace stmlib;

void Envelope::Init() {
  fill(&shape_[0], &shape_[kMaxNumSegments], ENV_SHAPE_LINEAR);
  set_ad(0, 8192);
  segment_ = num_segments_;
  phase_ = 0;
  phase_increment_ = 0;
  start_value_ = 0;
  value_ = 0;
  rate_modulation_ = 0;
  gate_level_ = 0;
  gate_ = false;
  hard_reset_ = false;
  attack_ = 0;
  decay_ = 0;
}

void Envelope::Process(
    int16_t audio,
    int16_t excite,
    uint16_t* gain,
    uint16_t* frequency) {
  // Smooth frequency amount parameters.
  frequency_amount_ += (target_frequency_amount_ - frequency_amount_) >> 8;
  frequency_offset_ += (target_frequency_offset_ - frequency_offset_) >> 8;

  bool trigger = false;
  bool release = false;
  if (gate_ == false) {
    if (excite > kSchmittTriggerThreshold) {
      trigger = true;
      gate_ = true;
      set_hard_reset(false);
    }
  } else {
    if (excite < (kSchmittTriggerThreshold >> 1)) {
      gate_ = false;
      release = false;
    } else {
      // Track the level of the signal while the GATE is held.
      gate_level_ += (excite - gate_level_) >> 8;
    }
  }
  if (trigger) {
    start_value_ = (segment_ == num_segments_ || hard_reset_)
        ? level_[0]
        : value_;
    segment_ = 0;
    phase_ = 0;
  } else if (release && sustain_point_) {
    start_value_ = value_;
    segment_ = sustain_point_;
    phase_ = 0;
  } else if (phase_ < phase_increment_) {
    start_value_ = level_[segment_ + 1];
    ++segment_;
    phase_ = 0;
  }
  
  bool done = segment_ == num_segments_;
  bool sustained = sustain_point_ && segment_ == sustain_point_ && gate_;
  uint32_t increment = sustained || done ? 0 : lut_env_increments[time_[segment_] >> 8];

  // Modulates the envelope rate by the actual excitation pulse.
  rate_modulation_ += (static_cast<int32_t>(excite > kSchmittTriggerThreshold ? excite : 0) - rate_modulation_) >> 12;
  increment += static_cast<int32_t>(increment >> 7) * (rate_modulation_ >> 7);
  
  phase_increment_ = increment;
  
  int32_t a = start_value_;
  int32_t b = level_[segment_ + 1];
  uint16_t t = Interpolate824(
      lookup_table_table[LUT_ENV_LINEAR + shape_[segment_]], phase_);
  value_ = a + ((b - a) * (t >> 1) >> 15);
  phase_ += phase_increment_;
  
  // Applies a variable amount of distortion, depending on the level.
  int32_t compressed = 32767 - ((32767 - value_) * (32767 - value_) >> 15);
  compressed = 32767 - ((32767 - compressed) * (32767 - compressed) >> 15);
  int32_t scaled = value_ + ((compressed - value_) * gate_level_ >> 15);
  scaled = scaled * (28672 + (gate_level_ >> 3)) >> 15;
  *gain = scaled * kAboveUnityGain >> 15;
  *frequency = frequency_offset_ + (scaled * frequency_amount_ >> 15);
}

}  // namespace streams
