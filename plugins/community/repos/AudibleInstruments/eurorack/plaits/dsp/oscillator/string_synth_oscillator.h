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
// A mixture of 7 sawtooth and square waveforms in the style of divide-down
// organs:
//
// [0]     [1]        [2]     [3]        [4]     [5]        [6]
// Saw 8', Square 8', Saw 4', Square 4', Saw 2', Square 2', Saw 1'
//
// Internally, this renders 4 band-limited sawtooths, from 8' to 1' from a
// single phase counter. The square waveforms are obtained by algebraic
// manipulations on the sawtooths, using the identity:
// Square 16' = 2 Sawtooth 16' - Sawtooth 8'

#ifndef PLAITS_DSP_OSCILLATOR_STRING_SYNTH_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_STRING_SYNTH_OSCILLATOR_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/polyblep.h"

namespace plaits {

class StringSynthOscillator {
 public:
  StringSynthOscillator() { }
  ~StringSynthOscillator() { }
  
  inline void Init() {
    phase_ = 0.0f;
    next_sample_ = 0.0f;
    segment_ = 0.0f;
    
    frequency_ = 0.001f;
    saw_8_gain_ = 0.0f;
    saw_4_gain_ = 0.0f;
    saw_2_gain_ = 0.0f;
    saw_1_gain_ = 0.0f;
  }
  
  inline void Render(
      float frequency,
      const float* unshifted_registration,
      float gain,
      float* out,
      size_t size) {
    frequency *= 8.0f;
    
    // Deal with very high frequencies by shifting everything 1 or 2 octave
    // down: Instead of playing the 1nd harmonic of a 8kHz wave, we play the
    // second harmonic of a 4kHz wave.
    size_t shift = 0;
    while (frequency > 0.5f) {
      shift += 2;
      frequency *= 0.5f;
    }
    // Frequency is just too high.
    if (shift >= 8) {
      return;
    }
    
    float registration[7];
    std::fill(&registration[0], &registration[shift], 0.0f);
    std::copy(
        &unshifted_registration[0],
        &unshifted_registration[7 - shift],
        &registration[shift]);
    
    stmlib::ParameterInterpolator fm(&frequency_, frequency, size);
    stmlib::ParameterInterpolator saw_8_gain_modulation(
        &saw_8_gain_,
        (registration[0] + 2.0f * registration[1]) * gain,
        size);
    stmlib::ParameterInterpolator saw_4_gain_modulation(
        &saw_4_gain_,
        (registration[2] - registration[1] + 2.0f * registration[3]) * gain,
        size);
    stmlib::ParameterInterpolator saw_2_gain_modulation(
        &saw_2_gain_,
        (registration[4] - registration[3] + 2.0f * registration[5]) * gain,
        size);
    stmlib::ParameterInterpolator saw_1_gain_modulation(
        &saw_1_gain_,
        (registration[6] - registration[5]) * gain,
        size);
    
    float phase = phase_;
    float next_sample = next_sample_;
    int segment = segment_;
    while (size--) {
      float this_sample = next_sample;
      next_sample = 0.0f;
  
      const float frequency = fm.Next();
      const float saw_8_gain = saw_8_gain_modulation.Next();
      const float saw_4_gain = saw_4_gain_modulation.Next();
      const float saw_2_gain = saw_2_gain_modulation.Next();
      const float saw_1_gain = saw_1_gain_modulation.Next();

      phase += frequency;
      int next_segment = static_cast<int>(phase);
      if (next_segment != segment) {
        float discontinuity = 0.0f;
        if (next_segment == 8) {
          phase -= 8.0f;
          next_segment -= 8;
          discontinuity -= saw_8_gain;
        }
        if ((next_segment & 3) == 0) {
          discontinuity -= saw_4_gain;
        }
        if ((next_segment & 1) == 0) {
          discontinuity -= saw_2_gain;
        }
        discontinuity -= saw_1_gain;
        if (discontinuity != 0.0f) {
          float fraction = phase - static_cast<float>(next_segment);
          float t = fraction / frequency;
          this_sample += stmlib::ThisBlepSample(t) * discontinuity;
          next_sample += stmlib::NextBlepSample(t) * discontinuity;
        }
      }
      segment = next_segment;
      
      next_sample += (phase - 4.0f) * saw_8_gain * 0.125f;
      next_sample += (phase - float(segment & 4) - 2.0f) * saw_4_gain * 0.25f;
      next_sample += (phase - float(segment & 6) - 1.0f) * saw_2_gain * 0.5f;
      next_sample += (phase - float(segment & 7) - 0.5f) * saw_1_gain;
      *out++ += 2.0f * this_sample;
    }
    next_sample_ = next_sample;
    phase_ = phase;
    segment_ = segment;
  }
 
 private:
  // Oscillator state.
  float phase_;
  float next_sample_;
  int segment_;

  // For interpolation of parameters.
  float frequency_;
  float saw_8_gain_;
  float saw_4_gain_;
  float saw_2_gain_;
  float saw_1_gain_;

  DISALLOW_COPY_AND_ASSIGN(StringSynthOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_STRING_SYNTH_OSCILLATOR_H_
