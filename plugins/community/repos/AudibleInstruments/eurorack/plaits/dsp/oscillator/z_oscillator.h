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
// Sinewave multiplied by and sync'ed to a carrier.

#ifndef PLAITS_DSP_OSCILLATOR_Z_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_Z_OSCILLATOR_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/polyblep.h"

#include "plaits/resources.h"

namespace plaits {

class ZOscillator {
 public:
  ZOscillator() { }
  ~ZOscillator() { }

  void Init() {
    carrier_phase_ = 0.0f;
    discontinuity_phase_ = 0.0f;
    formant_phase_ = 0.0f;
    next_sample_ = 0.0f;
  
    carrier_frequency_ = 0.0f;
    formant_frequency_ = 0.0f;
    carrier_shape_ = 0.0f;
    mode_ = 0.0f;
  }
  
  void Render(
      float carrier_frequency,
      float formant_frequency,
      float carrier_shape,
      float mode,
      float* out,
      size_t size) {
    if (carrier_frequency >= kMaxFrequency * 0.5f) {
      carrier_frequency = kMaxFrequency * 0.5f;
    }
    if (formant_frequency >= kMaxFrequency) {
      formant_frequency = kMaxFrequency;
    }
    
    stmlib::ParameterInterpolator carrier_frequency_modulation(
        &carrier_frequency_,
        carrier_frequency,
        size);
    stmlib::ParameterInterpolator formant_frequency_modulation(
        &formant_frequency_,
        formant_frequency,
        size);
    stmlib::ParameterInterpolator carrier_shape_modulation(
        &carrier_shape_,
        carrier_shape,
        size);
    stmlib::ParameterInterpolator mode_modulation(
        &mode_,
        mode,
        size);

    float next_sample = next_sample_;
    
    while (size--) {
      bool reset = false;
      float reset_time = 0.0f;

      float this_sample = next_sample;
      next_sample = 0.0f;
    
      const float f0 = carrier_frequency_modulation.Next();
      const float f1 = formant_frequency_modulation.Next();
    
      discontinuity_phase_ += 2.0f * f0;
      carrier_phase_ += f0;
      reset = discontinuity_phase_ >= 1.0f;
      
      if (reset) {
        discontinuity_phase_ -= 1.0f;
        reset_time = discontinuity_phase_ / (2.0f * f0);
        
        float carrier_phase_before = carrier_phase_ >= 1.0f ? 1.0f : 0.5f;
        float carrier_phase_after = carrier_phase_ >= 1.0f ? 0.0f : 0.5f;
        float before = Z(
            carrier_phase_before,
            1.0f,
            formant_phase_ + (1.0f - reset_time) * f1,
            carrier_shape_modulation.subsample(1.0f - reset_time),
            mode_modulation.subsample(1.0f - reset_time));

        float after = Z(
            carrier_phase_after,
            0.0f,
            0.0f,
            carrier_shape_modulation.subsample(1.0f),
            mode_modulation.subsample(1.0f));

        float discontinuity = after - before;
        this_sample += discontinuity * stmlib::ThisBlepSample(reset_time);
        next_sample += discontinuity * stmlib::NextBlepSample(reset_time);
        formant_phase_ = reset_time * f1;
        
        if (carrier_phase_ > 1.0f) {
          carrier_phase_ = discontinuity_phase_ * 0.5f;
        }
      } else {
        formant_phase_ += f1;
        if (formant_phase_ >= 1.0f) {
          formant_phase_ -= 1.0f;
        }
      }
      
      if (carrier_phase_ >= 1.0f) {
        carrier_phase_ -= 1.0f;
      }
      
      next_sample += Z(
          carrier_phase_,
          discontinuity_phase_,
          formant_phase_,
          carrier_shape_modulation.Next(),
          mode_modulation.Next());
      *out++ = this_sample;
    }
    
    next_sample_ = next_sample;
  }

 private:
  inline float Sine(float phase) {
    return stmlib::InterpolateWrap(lut_sine, phase, 1024.0f);
  }

  inline float Z(float c, float d, float f, float shape, float mode) {
    float ramp_down = 0.5f * (1.0f + Sine(0.5f * d + 0.25f));
    
    float offset;
    float phase_shift;
    if (mode < 0.333f) {
      offset = 1.0f;
      phase_shift = 0.25f + mode * 1.50f;
    } else if (mode < 0.666f) {
      phase_shift = 0.7495f - (mode - 0.33f) * 0.75f;
      offset = -Sine(phase_shift);
    } else {
      phase_shift = 0.7495f - (mode - 0.33f) * 0.75f;
      offset = 0.001f;
    }
    
    float discontinuity = Sine(f + phase_shift);
    float contour;
    if (shape < 0.5f) {
      shape *= 2.0f;
      if (c >= 0.5f) {
        ramp_down *= shape;
      }
      contour = 1.0f + (Sine(c + 0.25f) - 1.0f) * shape;
    } else {
      contour = Sine(c + shape * 0.5f);
    }
    return (ramp_down * (offset + discontinuity) - offset) * contour;
  }

  // Oscillator state.
  float carrier_phase_;
  float discontinuity_phase_;
  float formant_phase_;
  float next_sample_;

  // For interpolation of parameters.
  float carrier_frequency_;
  float formant_frequency_;
  float carrier_shape_;
  float mode_;
  
  DISALLOW_COPY_AND_ASSIGN(ZOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_Z_OSCILLATOR_H_
