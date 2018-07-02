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
// Polyblep oscillator used for string synth synthesis.

#ifndef RINGS_DSP_STRING_SYNTH_OSCILLATOR_H_
#define RINGS_DSP_STRING_SYNTH_OSCILLATOR_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/units.h"

namespace rings {

using namespace stmlib;

enum OscillatorShape {
  OSCILLATOR_SHAPE_BRIGHT_SQUARE,
  OSCILLATOR_SHAPE_SQUARE,
  OSCILLATOR_SHAPE_DARK_SQUARE,
  OSCILLATOR_SHAPE_TRIANGLE,
};

class StringSynthOscillator {
 public:
  StringSynthOscillator() { }
  ~StringSynthOscillator() { }

  inline void Init() {
    phase_ = 0.0f;
    phase_increment_ = 0.01f;
    filter_state_ = 0.0f;
    high_ = false;

    next_sample_ = 0.0f;
    next_sample_saw_ = 0.0f;

    gain_ = 0.0f;
    gain_saw_ = 0.0f;
  }
  
  template<OscillatorShape shape, bool interpolate_pitch>
  inline void Render(
      float target_increment,
      float target_gain,
      float target_gain_saw,
      float* out,
      size_t size) {
    // Cut harmonics above 12kHz, and low-pass harmonics above 8kHz to clear
    // highs
    if (target_increment >= 0.17f) {
      target_gain *= 1.0f - (target_increment - 0.17f) * 12.5f;
      if (target_increment >= 0.25f) {
        return;
      }
    }
    float phase = phase_;
    ParameterInterpolator phase_increment(
        &phase_increment_,
        target_increment,
        size);
    ParameterInterpolator gain(&gain_, target_gain, size);
    ParameterInterpolator gain_saw(&gain_saw_, target_gain_saw, size);

    float next_sample = next_sample_;
    float next_sample_saw = next_sample_saw_;
    float filter_state = filter_state_;
    bool high = high_;

    while (size--) {
      float this_sample = next_sample;
      float this_sample_saw = next_sample_saw;
      next_sample = 0.0f;
      next_sample_saw = 0.0f;

      float increment = interpolate_pitch
          ? phase_increment.Next()
          : target_increment;
      phase += increment;
    
      float sample = 0.0f;
      const float pw = 0.5f;

      if (!high && phase >= pw) {
        float t = (phase - pw) / increment;
        this_sample += ThisBlepSample(t);
        next_sample += NextBlepSample(t);
        high = true;
      }
      if (phase >= 1.0f) {
        phase -= 1.0f;
        float t = phase / increment;
        float a = ThisBlepSample(t);
        float b = NextBlepSample(t);
        this_sample -= a;
        next_sample -= b;
        this_sample_saw -= a;
        next_sample_saw -= b;
        high = false;
      }
      
      next_sample += phase < pw ? 0.0f : 1.0f;
      next_sample_saw += phase;
      
      if (shape == OSCILLATOR_SHAPE_TRIANGLE) {
        const float integrator_coefficient = increment * 0.125f;
        this_sample = 64.0f * (this_sample - 0.5f);
        filter_state += integrator_coefficient * (this_sample - filter_state);
        sample = filter_state;
      } else if (shape == OSCILLATOR_SHAPE_DARK_SQUARE) {
        const float integrator_coefficient = increment * 2.0f;
        this_sample = 4.0f * (this_sample - 0.5f);
        filter_state += integrator_coefficient * (this_sample - filter_state);
        sample = filter_state;
      } else if (shape == OSCILLATOR_SHAPE_BRIGHT_SQUARE) {
        const float integrator_coefficient = increment * 2.0f;
        this_sample = 2.0f * this_sample - 1.0f;
        filter_state += integrator_coefficient * (this_sample - filter_state);
        sample = (this_sample - filter_state) * 0.5f;
      } else {
        this_sample = 2.0f * this_sample - 1.0f;
        sample = this_sample;
      }
      this_sample_saw = 2.0f * this_sample_saw - 1.0f;
      
      *out++ += sample * gain.Next() + this_sample_saw * gain_saw.Next();
    }
    high_ = high;
    phase_ = phase;
    next_sample_ = next_sample;
    next_sample_saw_ = next_sample_saw;
    filter_state_ = filter_state;
  }

 private:
  static inline float ThisBlepSample(float t) {
    return 0.5f * t * t;
  }
  static inline float NextBlepSample(float t) {
    t = 1.0f - t;
    return -0.5f * t * t;
  }

  bool high_;
  float phase_;
  float phase_increment_;
  float next_sample_;
  float next_sample_saw_;
  float filter_state_;
  float gain_;
  float gain_saw_;

  DISALLOW_COPY_AND_ASSIGN(StringSynthOscillator);
};

}  // namespace rings

#endif  // RINGS_DSP_STRING_SYNTH_OSCILLATOR_H_
