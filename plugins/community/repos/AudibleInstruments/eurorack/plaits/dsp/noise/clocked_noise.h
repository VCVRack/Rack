// Copyright 2016 Olivier Gillet.
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
// Noise processed by a sample and hold running at a target frequency.

#ifndef PLAITS_DSP_NOISE_CLOCKED_NOISE_H_
#define PLAITS_DSP_NOISE_CLOCKED_NOISE_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/polyblep.h"
#include "stmlib/utils/random.h"

namespace plaits {

class ClockedNoise {
 public:
  ClockedNoise() { }
  ~ClockedNoise() { }
  
  void Init() {
    phase_ = 0.0f;
    sample_ = 0.0f;
    next_sample_ = 0.0f;
    frequency_ = 0.001f;
  }

  void Render(bool sync, float frequency, float* out, size_t size) {
    CONSTRAIN(frequency, 0.0f, 1.0f);
    
    stmlib::ParameterInterpolator fm(&frequency_, frequency, size);

    float next_sample = next_sample_;
    float sample = sample_;
    
    if (sync) {
      phase_ = 1.0f;
    }

    while (size--) {
      float this_sample = next_sample;
      next_sample = 0.0f;

      const float frequency = fm.Next();
      const float raw_sample = stmlib::Random::GetFloat() * 2.0f - 1.0f;
      float raw_amount = 4.0f * (frequency - 0.25f);
      CONSTRAIN(raw_amount, 0.0f, 1.0f);
      
      phase_ += frequency;
      
      if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
        float t = phase_ / frequency;
        float new_sample = raw_sample;
        float discontinuity = new_sample - sample;
        this_sample += discontinuity * stmlib::ThisBlepSample(t);
        next_sample += discontinuity * stmlib::NextBlepSample(t);
        sample = new_sample;
      }
      next_sample += sample;
      *out++ = this_sample + raw_amount * (raw_sample - this_sample);
    }
    next_sample_ = next_sample;
    sample_ = sample;
  }
  
 private:
  // Oscillator state.
  float phase_;
  float sample_;
  float next_sample_;

  // For interpolation of parameters.
  float frequency_;
  
  DISALLOW_COPY_AND_ASSIGN(ClockedNoise);
};

}  // namespace plaits

#endif  // PLAITS_DSP_NOISE_CLOCKED_NOISE_H_
