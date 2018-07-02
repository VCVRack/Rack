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
// Envelope / centroid follower for FM voice.

#ifndef RINGS_DSP_FOLLOWER_H_
#define RINGS_DSP_FOLLOWER_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

namespace rings {

using namespace stmlib;

class Follower {
 public:  
  Follower() { }
  ~Follower() { }
  
  void Init(float low, float low_mid, float mid_high) {
    low_mid_filter_.Init();
    mid_high_filter_.Init();
    
    low_mid_filter_.set_f_q<FREQUENCY_DIRTY>(low_mid, 0.5f);
    mid_high_filter_.set_f_q<FREQUENCY_DIRTY>(mid_high, 0.5f);
    attack_[0] = low_mid;
    decay_[0] = Sqrt(low_mid * low);

    attack_[1] = Sqrt(low_mid * mid_high);
    decay_[1] = low_mid;

    attack_[2] = Sqrt(mid_high * 0.5f);
    decay_[2] = Sqrt(mid_high * low_mid);

    std::fill(&detector_[0], &detector_[3], 0.0f);
    
    centroid_ = 0.0f;
  }

  void Process(
      float sample,
      float* envelope,
      float* centroid) {
    float bands[3] = { 0.0f, 0.0f, 0.0f };
    
    bands[2] = mid_high_filter_.Process<FILTER_MODE_HIGH_PASS>(sample);
    bands[1] = low_mid_filter_.Process<FILTER_MODE_HIGH_PASS>(
        mid_high_filter_.lp());
    bands[0] = low_mid_filter_.lp();
    
    float weighted = 0.0f;
    float total = 0.0f;
    float frequency = 0.0f;
    for (int32_t i = 0; i < 3; ++i) {
      SLOPE(detector_[i], fabs(bands[i]), attack_[i], decay_[i]);
      weighted += detector_[i] * frequency;
      total += detector_[i];
      frequency += 0.5f;
    }
    
    float error = weighted / (total + 0.001f) - centroid_;
    float coefficient = error > 0.0f ? 0.05f : 0.001f;
    centroid_ += error * coefficient;
    
    *envelope = total;
    *centroid = centroid_;
  }
  
 private:
  NaiveSvf low_mid_filter_;
  NaiveSvf mid_high_filter_;
  
  float attack_[3];
  float decay_[3];
  float detector_[3];
  
  float centroid_;
  
  DISALLOW_COPY_AND_ASSIGN(Follower);
};

}  // namespace rings

#endif  // RINGS_DSP_FOLLOWER_H_
