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
// Oscillator.

#ifndef WARPS_DSP_OSCILLATOR_H_
#define WARPS_DSP_OSCILLATOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

#include "warps/dsp/parameters.h"
#include "warps/resources.h"

namespace warps {

const float kInternalOscillatorSampleRate = 96000.0f;

class Oscillator {
 public:
  Oscillator() { }
  ~Oscillator() { }
  
  void Init(float sample_rate); 
  
  float Render(
      OscillatorShape shape,
      float note,
      float* modulation,
      float* out,
      size_t size);

  inline float midi_to_increment(float midi_pitch) const {
    int32_t pitch = static_cast<int32_t>(midi_pitch * 256.0f);
    pitch = 32768 + stmlib::Clip16(pitch - 20480);
    float increment = lut_midi_to_f_high[pitch >> 8] * \
        lut_midi_to_f_low[pitch & 0xff];
    return increment;
  }
  
  typedef float (Oscillator::*RenderFn)(
      float note, float* mod, float* out, size_t size);

 private:
  float Duck(
      const float* internal,
      const float* external,
      float* destination,
      size_t size);

  template<OscillatorShape shape>
  float RenderPolyblep(float, float*, float*, size_t);
  float RenderSine(float, float*, float*, size_t);
  float RenderNoise(float, float*, float*, size_t);
  
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
  float lp_state_;
  float hp_state_;
  
  float external_input_level_;
  float one_hertz_;
  
  static RenderFn fn_table_[];
  stmlib::Svf filter_;

  DISALLOW_COPY_AND_ASSIGN(Oscillator);
};

}  // namespace warps

#endif  // WARPS_DSP_OSCILLATOR_H_
