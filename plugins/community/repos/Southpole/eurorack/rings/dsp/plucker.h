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
// Noise burst generator for Karplus-Strong synthesis.

#ifndef RINGS_DSP_PLUCKER_H_
#define RINGS_DSP_PLUCKER_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/filter.h"
#include "stmlib/dsp/delay_line.h"
#include "stmlib/utils/random.h"

namespace rings {

class Plucker {
 public:
  Plucker() { }
  ~Plucker() { }
  
  void Init() {
    svf_.Init();
    comb_filter_.Init();
    remaining_samples_ = 0;
    comb_filter_period_ = 0.0f;
  }
  
  void Trigger(float frequency, float cutoff, float position) {
    float ratio = position * 0.9f + 0.05f;
    float comb_period = 1.0f / frequency * ratio;
    remaining_samples_ = static_cast<size_t>(comb_period);
    while (comb_period >= 255.0f) {
      comb_period *= 0.5f;
    }
    comb_filter_period_ = comb_period;
    comb_filter_gain_ = (1.0f - position) * 0.8f;
    svf_.set_f_q<FREQUENCY_DIRTY>(std::min(cutoff, 0.499f), 1.0f);
  }
  
  void Process(float* out, size_t size) {
    const float comb_gain = comb_filter_gain_;
    const float comb_delay = comb_filter_period_;
    for (size_t i = 0; i < size; ++i) {
      float in = 0.0f;
      if (remaining_samples_) {
        in = 2.0f * Random::GetFloat() - 1.0f;
        --remaining_samples_;
      }
      out[i] = in + comb_gain * comb_filter_.Read(comb_delay);
      comb_filter_.Write(out[i]);
    }
    svf_.Process<FILTER_MODE_LOW_PASS>(out, out, size);
  }

 private:
  stmlib::Svf svf_;
  stmlib::DelayLine<float, 256> comb_filter_;
  size_t remaining_samples_;
  float comb_filter_period_;
  float comb_filter_gain_;
  
  DISALLOW_COPY_AND_ASSIGN(Plucker);
};

}  // namespace rings

#endif  // RINGS_DSP_PLUCKER_H_
