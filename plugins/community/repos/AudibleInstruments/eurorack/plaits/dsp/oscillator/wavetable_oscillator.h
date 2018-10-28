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
// Integrated wavetable synthesis.

#ifndef PLAITS_DSP_OSCILLATOR_WAVETABLE_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_WAVETABLE_OSCILLATOR_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include "plaits/dsp/oscillator/oscillator.h"

namespace plaits {

class Differentiator {
 public:
  Differentiator() { }
  ~Differentiator() { }

  void Init() {
    previous_ = 0.0f;
    lp_ = 0.0f;
  }
  
  float Process(float coefficient, float s) {
    ONE_POLE(lp_, s - previous_, coefficient);
    previous_ = s;
    return lp_;
  }
 private:
  float lp_;
  float previous_;

  DISALLOW_COPY_AND_ASSIGN(Differentiator);
};

inline float InterpolateWave(
    const int16_t* table,
    int32_t index_integral,
    float index_fractional) {
  float a = static_cast<float>(table[index_integral]);
  float b = static_cast<float>(table[index_integral + 1]);
  float t = index_fractional;
  return a + (b - a) * t;
}

inline float InterpolateWaveHermite(
    const int16_t* table,
    int32_t index_integral,
    float index_fractional) {
  const float xm1 = table[index_integral];
  const float x0 = table[index_integral + 1];
  const float x1 = table[index_integral + 2];
  const float x2 = table[index_integral + 3];
  const float c = (x1 - xm1) * 0.5f;
  const float v = x0 - x1;
  const float w = c + v;
  const float a = w + v + (x2 - x0) * 0.5f;
  const float b_neg = w + a;
  const float f = index_fractional;
  return (((a * f) - b_neg) * f + c) * f + x0;
}

template<
    size_t wavetable_size,
    size_t num_waves,
    bool approximate_scale=true>
class WavetableOscillator {
 public:
  WavetableOscillator() { }
  ~WavetableOscillator() { }

  void Init() {
    phase_ = 0.0f;
    frequency_ = 0.0f;
    amplitude_ = 0.0f;
    waveform_ = 0.0f;
    lp_ = 0.0f;
    differentiator_.Init();
  }
  
  void Render(
      float frequency,
      float amplitude,
      float waveform,
      const int16_t** wavetable,
      float* out,
      size_t size) {
    if (frequency >= kMaxFrequency) {
      frequency = kMaxFrequency;
    }
    amplitude *= 1.0f - 2.0f * frequency;
    if (approximate_scale) {
      amplitude *= 1.0f / (frequency * 131072.0f) * (0.95f - frequency);
    }

    stmlib::ParameterInterpolator frequency_modulation(
        &frequency_,
        frequency,
        size);
    stmlib::ParameterInterpolator amplitude_modulation(
        &amplitude_,
        amplitude,
        size);
    stmlib::ParameterInterpolator waveform_modulation(
        &waveform_,
        waveform * float(num_waves - 1.0001f),
        size);
    
    float lp = lp_;
    float phase = phase_;
    while (size--) {
      const float f0 = frequency_modulation.Next();
      const float cutoff = std::min(float(wavetable_size) * f0, 1.0f);
      
      const float scale = approximate_scale ? 1.0f : 1.0f / (f0 * 131072.0f) * (0.95f - f0);
      
      phase += f0;
      if (phase >= 1.0f) {
        phase -= 1.0f;
      }
      
      const float waveform = waveform_modulation.Next();
      MAKE_INTEGRAL_FRACTIONAL(waveform);
      
      const float p = phase * float(wavetable_size);
      MAKE_INTEGRAL_FRACTIONAL(p);
      
      const float x0 = InterpolateWave(
          wavetable[waveform_integral], p_integral, p_fractional);
      const float x1 = InterpolateWave(
          wavetable[waveform_integral + 1], p_integral, p_fractional);
      
      const float s = differentiator_.Process(
          cutoff,
          x0 + (x1 - x0) * waveform_fractional);
      ONE_POLE(lp, s * scale, cutoff * 0.5f);
      *out++ += amplitude_modulation.Next() * lp;
    }
    lp_ = lp;
    phase_ = phase;
  }

 private:
  // Oscillator state.
  float phase_;

  // For interpolation of parameters.
  float frequency_;
  float amplitude_;
  float waveform_;
  float lp_;
  
  Differentiator differentiator_;
  
  DISALLOW_COPY_AND_ASSIGN(WavetableOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_WAVETABLE_OSCILLATOR_H_
