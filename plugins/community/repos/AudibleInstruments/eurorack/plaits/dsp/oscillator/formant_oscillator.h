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
// Sinewave with aliasing-free phase reset.

#ifndef PLAITS_DSP_OSCILLATOR_FORMANT_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_FORMANT_OSCILLATOR_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/polyblep.h"

#include "plaits/resources.h"

namespace plaits {

class FormantOscillator {
 public:
  FormantOscillator() { }
  ~FormantOscillator() { }

  void Init() {
    carrier_phase_ = 0.0f;
    formant_phase_ = 0.0f;
    next_sample_ = 0.0f;
  
    carrier_frequency_ = 0.0f;
    formant_frequency_ = 0.01f;
    phase_shift_ = 0.0f;
  }
  
  void Render(
      float carrier_frequency,
      float formant_frequency,
      float phase_shift,
      float* out,
      size_t size) {
    if (carrier_frequency >= kMaxFrequency) {
      carrier_frequency = kMaxFrequency;
    }
    if (formant_frequency >= kMaxFrequency) {
      formant_frequency = kMaxFrequency;
    }

    stmlib::ParameterInterpolator carrier_fm(
        &carrier_frequency_, carrier_frequency, size);
    stmlib::ParameterInterpolator formant_fm(
        &formant_frequency_, formant_frequency, size);
    stmlib::ParameterInterpolator pm(&phase_shift_, phase_shift, size);

    float next_sample = next_sample_;
    
    while (size--) {
      float this_sample = next_sample;
      next_sample = 0.0f;
    
      const float carrier_frequency = carrier_fm.Next();
      const float formant_frequency = formant_fm.Next();
    
      carrier_phase_ += carrier_frequency;
      
      if (carrier_phase_ >= 1.0f) {
        carrier_phase_ -= 1.0f;
        float reset_time = carrier_phase_ / carrier_frequency;
    
        float formant_phase_at_reset = formant_phase_ + \
            (1.0f - reset_time) * formant_frequency;
        float before = Sine(
            formant_phase_at_reset + pm.subsample(1.0f - reset_time));
        float after = Sine(0.0f + pm.subsample(1.0f));
        float discontinuity = after - before;
        this_sample += discontinuity * stmlib::ThisBlepSample(reset_time);
        next_sample += discontinuity * stmlib::NextBlepSample(reset_time);
        formant_phase_ = reset_time * formant_frequency;
      } else {
        formant_phase_ += formant_frequency;
        if (formant_phase_ >= 1.0f) {
          formant_phase_ -= 1.0f;
        }
      }
    
      const float phase_shift = pm.Next();
      next_sample += Sine(formant_phase_ + phase_shift);

      *out++ = this_sample;
    }
    next_sample_ = next_sample;
  }

 private:
  inline float Sine(float phase) {
    return stmlib::InterpolateWrap(lut_sine, phase, 1024.0f);
  }

  // Oscillator state.
  float carrier_phase_;
  float formant_phase_;
  float next_sample_;

  // For interpolation of parameters.
  float carrier_frequency_;
  float formant_frequency_;
  float phase_shift_;
  
  DISALLOW_COPY_AND_ASSIGN(FormantOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_FORMANT_OSCILLATOR_H_
