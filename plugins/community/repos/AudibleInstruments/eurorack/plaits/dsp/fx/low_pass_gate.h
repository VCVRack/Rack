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
// Approximative low pass gate.

#ifndef PLAITS_DSP_FX_LOW_PASS_GATE_H_
#define PLAITS_DSP_FX_LOW_PASS_GATE_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"
#include "stmlib/dsp/parameter_interpolator.h"

namespace plaits {
  
class LowPassGate {
 public:
  LowPassGate() { }
  ~LowPassGate() { }
  
  void Init() {
    previous_gain_ = 0.0f;
    filter_.Init();
  }
  
  void Process(
      float gain,
      float frequency,
      float hf_bleed,
      float* in_out,
      size_t size) {
    stmlib::ParameterInterpolator gain_modulation(&previous_gain_, gain, size);
    filter_.set_f_q<stmlib::FREQUENCY_DIRTY>(frequency, 0.4f);
    while (size--) {
      const float s = *in_out * gain_modulation.Next();
      const float lp = filter_.Process<stmlib::FILTER_MODE_LOW_PASS>(s);
      *in_out++ = lp + (s - lp) * hf_bleed;
    }
  }
  
  void Process(
      float gain,
      float frequency,
      float hf_bleed,
      float* in,
      short* out,
      size_t size,
      size_t stride) {
    stmlib::ParameterInterpolator gain_modulation(&previous_gain_, gain, size);
    filter_.set_f_q<stmlib::FREQUENCY_DIRTY>(frequency, 0.4f);
    while (size--) {
      const float s = *in++ * gain_modulation.Next();
      const float lp = filter_.Process<stmlib::FILTER_MODE_LOW_PASS>(s);
      *out = stmlib::Clip16(1 + static_cast<int32_t>(lp + (s - lp) * hf_bleed));
      out += stride;
    }
  }
  
 private:
  float previous_gain_;
  stmlib::Svf filter_;
  
  DISALLOW_COPY_AND_ASSIGN(LowPassGate);
};

}  // namespace plaits

#endif  // PLAITS_DSP_FX_LOW_PASS_GATE_H_
