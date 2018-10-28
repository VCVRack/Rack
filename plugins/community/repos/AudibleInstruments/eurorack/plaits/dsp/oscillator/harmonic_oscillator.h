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
// Harmonic oscillator based on Chebyshev polynomials.
// Works well for a small number of harmonics. For the higher order harmonics,
// we need to reinitialize the recurrence by computing two high harmonics.

#ifndef PLAITS_DSP_OSCILLATOR_HARMONIC_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_HARMONIC_OSCILLATOR_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include "plaits/resources.h"

namespace plaits {

template<int num_harmonics>
class HarmonicOscillator {
 public:
  HarmonicOscillator() { }
  ~HarmonicOscillator() { }

  void Init() {
    phase_ = 0.0f;
    frequency_ = 0.0f;
    for (int i = 0; i < num_harmonics; ++i) {
      amplitude_[i] = 0.0f;
    }
  }
  
  template<int first_harmonic_index>
  void Render(
      float frequency,
      const float* amplitudes,
      float* out,
      size_t size) {
    if (frequency >= 0.5f) {
      frequency = 0.5f;
    }
    
    stmlib::ParameterInterpolator am[num_harmonics];
    stmlib::ParameterInterpolator fm(&frequency_, frequency, size);
    
    for (int i = 0; i < num_harmonics; ++i) {
      float f = frequency * static_cast<float>(first_harmonic_index + i);
      if (f >= 0.5f) {
        f = 0.5f;
      }
      am[i].Init(&amplitude_[i], amplitudes[i] * (1.0f - f * 2.0f), size);
    }

    while (size--) {
      phase_ += fm.Next();
      if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
      }
      const float two_x = 2.0f * stmlib::Interpolate(lut_sine, phase_, 1024.0f);
      float previous, current;
      if (first_harmonic_index == 1) {
        previous = 1.0f;
        current = two_x * 0.5f;
      } else {
        const float k = first_harmonic_index;
        previous = stmlib::InterpolateWrap(
            lut_sine, phase_ * (k - 1.0f) + 0.25f, 1024.0f);
        current = stmlib::InterpolateWrap(lut_sine, phase_ * k, 1024.0f);
      }
      
      float sum = 0.0f;
      for (int i = 0; i < num_harmonics; ++i) {
        sum += am[i].Next() * current;
        float temp = current;
        current = two_x * current - previous;
        previous = temp;
      }
      if (first_harmonic_index == 1) {
        *out++ = sum;
      } else {
        *out++ += sum;
      }
    }
  }

 private:
  // Oscillator state.
  float phase_;

  // For interpolation of parameters.
  float frequency_;
  float amplitude_[num_harmonics];
  
  DISALLOW_COPY_AND_ASSIGN(HarmonicOscillator);
};

}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_HARMONIC_OSCILLATOR_H_
