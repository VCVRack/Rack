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
// Distortion/overdrive.

#ifndef PLAITS_DSP_FX_OVERDRIVE_H_
#define PLAITS_DSP_FX_OVERDRIVE_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"

namespace plaits {
  
class Overdrive {
 public:
  Overdrive() { }
  ~Overdrive() { }
  
  void Init() {
    pre_gain_ = 0.0f;
    post_gain_ = 0.0f;
  }
  
  void Process(float drive, float* in_out, size_t size) {
    const float drive_2 = drive * drive;
    const float pre_gain_a = drive * 0.5f;
    const float pre_gain_b = drive_2 * drive_2 * drive * 24.0f;
    const float pre_gain = pre_gain_a + (pre_gain_b - pre_gain_a) * drive_2;
    const float drive_squashed = drive * (2.0f - drive);
    const float post_gain = 1.0f / stmlib::SoftClip(
          0.33f + drive_squashed * (pre_gain - 0.33f));
    
    stmlib::ParameterInterpolator pre_gain_modulation(
        &pre_gain_,
        pre_gain,
        size);
    
    stmlib::ParameterInterpolator post_gain_modulation(
        &post_gain_,
        post_gain,
        size);
    
    while (size--) {
      float pre = pre_gain_modulation.Next() * *in_out;
      *in_out++ = stmlib::SoftClip(pre) * post_gain_modulation.Next();
    }
  }
  
 private:
  float pre_gain_;
  float post_gain_;
  
  DISALLOW_COPY_AND_ASSIGN(Overdrive);
};

}  // namespace plaits

#endif  // PLAITS_DSP_FX_OVERDRIVE_H_
