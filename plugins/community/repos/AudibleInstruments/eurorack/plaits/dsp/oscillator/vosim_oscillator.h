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
// Two sinewaves multiplied by and sync'ed to a carrier.

#ifndef PLAITS_DSP_OSCILLATOR_VOSIM_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_VOSIM_OSCILLATOR_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include "plaits/dsp/oscillator/oscillator.h"
#include "plaits/resources.h"

namespace plaits {

class VOSIMOscillator {
 public:
  VOSIMOscillator() { }
  ~VOSIMOscillator() { }

  void Init() {
    carrier_phase_ = 0.0f;
    formant_1_phase_ = 0.0f;
    formant_2_phase_ = 0.0f;
  
    carrier_frequency_ = 0.0f;
    formant_1_frequency_ = 0.0f;
    formant_2_frequency_ = 0.0f;
    carrier_shape_ = 0.0f;
  }
  
  void Render(
      float carrier_frequency,
      float formant_frequency_1,
      float formant_frequency_2,
      float carrier_shape,
      float* out,
      size_t size) {
    if (carrier_frequency >= kMaxFrequency) {
      carrier_frequency = kMaxFrequency;
    }
    if (formant_frequency_1 >= kMaxFrequency) {
      formant_frequency_1 = kMaxFrequency;
    }
    if (formant_frequency_2 >= kMaxFrequency) {
      formant_frequency_2 = kMaxFrequency;
    }

    stmlib::ParameterInterpolator f0_modulation(
        &carrier_frequency_,
        carrier_frequency,
        size);
    stmlib::ParameterInterpolator f1_modulation(
        &formant_1_frequency_,
        formant_frequency_1,
        size);
    stmlib::ParameterInterpolator f2_modulation(
        &formant_2_frequency_,
        formant_frequency_2,
        size);
    stmlib::ParameterInterpolator carrier_shape_modulation(
        &carrier_shape_,
        carrier_shape,
        size);

    while (size--) {
      const float f0 = f0_modulation.Next();
      const float f1 = f1_modulation.Next();
      const float f2 = f2_modulation.Next();
    
      carrier_phase_ += carrier_frequency;
      if (carrier_phase_ >= 1.0f) {
        carrier_phase_ -= 1.0f;
        float reset_time = carrier_phase_ / f0;
        formant_1_phase_ = reset_time * f1;
        formant_2_phase_ = reset_time * f2;
      } else {
        formant_1_phase_ += f1;
        if (formant_1_phase_ >= 1.0f) {
          formant_1_phase_ -= 1.0f;
        }
        formant_2_phase_ += f2;
        if (formant_2_phase_ >= 1.0f) {
          formant_2_phase_ -= 1.0f;
        }
      }
      
      float carrier = Sine(carrier_phase_ * 0.5f + 0.25f) + 1.0f;
      float reset_phase = 0.75f - 0.25f * carrier_shape_modulation.Next();
      float reset_amplitude = Sine(reset_phase);
      float formant_0 = Sine(formant_1_phase_ + reset_phase) - reset_amplitude;
      float formant_1 = Sine(formant_2_phase_ + reset_phase) - reset_amplitude;
      *out++ = carrier * (formant_0 + formant_1) * 0.25f + reset_amplitude;
    }
  }

 private:
  inline float Sine(float phase) {
    return stmlib::InterpolateWrap(lut_sine, phase, 1024.0f);
  }

  // Oscillator state.
  float carrier_phase_;
  float formant_1_phase_;
  float formant_2_phase_;

  // For interpolation of parameters.
  float carrier_frequency_;
  float formant_1_frequency_;
  float formant_2_frequency_;
  float carrier_shape_;
  
  DISALLOW_COPY_AND_ASSIGN(VOSIMOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_VOSIM_OSCILLATOR_H_
