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
// Pitch shifter.

#ifndef CLOUDS_DSP_FX_PITCH_SHIFTER_H_
#define CLOUDS_DSP_FX_PITCH_SHIFTER_H_

#include "stmlib/stmlib.h"

#include "clouds/dsp/frame.h"
#include "clouds/dsp/fx/fx_engine.h"

namespace clouds {

class PitchShifter {
 public:
  PitchShifter() { }
  ~PitchShifter() { }
  
  void Init(uint16_t* buffer) {
    engine_.Init(buffer);
    phase_ = 0;
    size_ = 2047.0f;
  }
  
  void Clear() {
    engine_.Clear();
  }

  inline void Process(FloatFrame* input_output, size_t size) {
    while (size--) {
      Process(input_output);
      ++input_output;
    }
  }
  
  void Process(FloatFrame* input_output) {
    typedef E::Reserve<2047, E::Reserve<2047> > Memory;
    E::DelayLine<Memory, 0> left;
    E::DelayLine<Memory, 1> right;
    E::Context c;
    engine_.Start(&c);
    
    phase_ += (1.0f - ratio_) / size_;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
    }
    if (phase_ <= 0.0f) {
      phase_ += 1.0f;
    }
    float tri = 2.0f * (phase_ >= 0.5f ? 1.0f - phase_ : phase_);
    float phase = phase_ * size_;
    float half = phase + size_ * 0.5f;
    if (half >= size_) {
      half -= size_;
    }
    
    c.Read(input_output->l, 1.0f);
    c.Write(left, 0.0f);
    c.Interpolate(left, phase, tri);
    c.Interpolate(left, half, 1.0f - tri);
    c.Write(input_output->l, 0.0f);

    c.Read(input_output->r, 1.0f);
    c.Write(right, 0.0f);
    c.Interpolate(right, phase, tri);
    c.Interpolate(right, half, 1.0f - tri);
    c.Write(input_output->r, 0.0f);
  }
  
  inline void set_ratio(float ratio) {
    ratio_ = ratio;
  }
  
  inline void set_size(float size) {
    float target_size = 128.0f + (2047.0f - 128.0f) * size * size * size;
    ONE_POLE(size_, target_size, 0.05f)
  }
  
 private:
  typedef FxEngine<4096, FORMAT_16_BIT> E;
  E engine_;
  float phase_;
  float ratio_;
  float size_;
  
  DISALLOW_COPY_AND_ASSIGN(PitchShifter);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_FX_MINI_CHORUS_H_