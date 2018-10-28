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
// Simple sine oscillator (wavetable) + fast sine oscillator (magic circle).
//
// The fast implementation might glitch a bit under heavy modulations of the
// frequency.

#ifndef PLAITS_DSP_OSCILLATOR_SINE_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_SINE_OSCILLATOR_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/rsqrt.h"

#include "plaits/resources.h"

namespace plaits {

class SineOscillator {
 public:
  SineOscillator() { }
  ~SineOscillator() { }

  void Init() {
    phase_ = 0.0f;
    frequency_ = 0.0f;
    amplitude_ = 0.0f;
  }
  
  inline float Next(float frequency) {
    if (frequency >= 0.5f) {
      frequency = 0.5f;
    }
    
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
    }
    
    return stmlib::Interpolate(lut_sine, phase_, 1024.0f);
  }
  
  inline void Next(float frequency, float amplitude, float* sin, float* cos) {
    if (frequency >= 0.5f) {
      frequency = 0.5f;
    }
    
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
    }
    
    *sin = amplitude * stmlib::Interpolate(lut_sine, phase_, 1024.0f);
    *cos = amplitude * stmlib::Interpolate(lut_sine + 256, phase_, 1024.0f);
  }
  
  void Render(float frequency, float amplitude, float* out, size_t size) {
    RenderInternal<true>(frequency, amplitude, out, size);
  }
  
  void Render(float frequency, float* out, size_t size) {
    RenderInternal<false>(frequency, 1.0f, out, size);
  }

 private:
  template<bool additive>
  void RenderInternal(
      float frequency, float amplitude, float* out, size_t size) {
    if (frequency >= 0.5f) {
      frequency = 0.5f;
    }
    stmlib::ParameterInterpolator fm(&frequency_, frequency, size);
    stmlib::ParameterInterpolator am(&amplitude_, amplitude, size);

    while (size--) {
      phase_ += fm.Next();
      if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
      }
      float s = stmlib::Interpolate(lut_sine, phase_, 1024.0f);
      if (additive) {
        *out++ += am.Next() * s;
      } else {
        *out++ = s;
      }
    }
  }
  
  // Oscillator state.
  float phase_;

  // For interpolation of parameters.
  float frequency_;
  float amplitude_;
  
  DISALLOW_COPY_AND_ASSIGN(SineOscillator);
};

class FastSineOscillator {
 public:
  FastSineOscillator() { }
  ~FastSineOscillator() { }

  void Init() {
    x_ = 1.0f;
    y_ = 0.0f;
    epsilon_ = 0.0f;
    amplitude_ = 0.0f;
  }
  
  static inline float Fast2Sin(float f) {
    // In theory, epsilon = 2 sin(pi f)
    // Here, to avoid the call to sinf, we use a 3rd order polynomial
    // approximation, which looks like a Taylor expansion, but with a
    // correction term to give a good trade-off between average error
    // (1.13 cents) and maximum error (7.33 cents) when generating sinewaves
    // in the 16 Hz to 16kHz range (with sr = 48kHz).
    const float f_pi = f * float(M_PI);
    return f_pi * (2.0f - (2.0f * 0.96f / 6.0f) * f_pi * f_pi);
  }
  
  void Render(float frequency, float* out, size_t size) {
    RenderInternal<false>(frequency, 1.0f, out, size);
  }
  
  void Render(float frequency, float amplitude, float* out, size_t size) {
    RenderInternal<true>(frequency, amplitude, out, size);
  }
  
 private:
  template<bool additive>
  void RenderInternal(
      float frequency, float amplitude, float* out, size_t size) {
    if (frequency >= 0.25f) {
      frequency = 0.25f;
      amplitude = 0.0f;
    } else {
      amplitude *= 1.0f - frequency * 4.0f;
    }
    
    stmlib::ParameterInterpolator epsilon(&epsilon_, Fast2Sin(frequency), size);
    stmlib::ParameterInterpolator am(&amplitude_, amplitude, size);
    float x = x_;
    float y = y_;
    
    const float norm = x * x + y * y;
    if (norm <= 0.5f || norm >= 2.0f) {
      const float scale = stmlib::fast_rsqrt_carmack(norm);
      x *= scale;
      y *= scale;
    }
    
    while (size--) {
      const float e = epsilon.Next();
      x += e * y;
      y -= e * x;
      if (additive) {
        *out++ += am.Next() * x;
      } else {
        *out++ = x;
      }
    }
    x_ = x;
    y_ = y;
  }
     
  // Oscillator state.
  float x_;
  float y_;

  // For interpolation of parameters.
  float epsilon_;
  float amplitude_;
  
  DISALLOW_COPY_AND_ASSIGN(FastSineOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_SINE_OSCILLATOR_H_
