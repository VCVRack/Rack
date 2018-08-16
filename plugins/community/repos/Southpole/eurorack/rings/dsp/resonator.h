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
// Resonator.

#ifndef RINGS_DSP_RESONATOR_H_
#define RINGS_DSP_RESONATOR_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "rings/dsp/dsp.h"
#include "stmlib/dsp/filter.h"
#include "stmlib/dsp/delay_line.h"

namespace rings {

const int32_t kMaxModes = 64;

class Resonator {
 public:
  Resonator() { }
  ~Resonator() { }
  
  void Init();
  void Process(
      const float* in,
      float* out,
      float* aux,
      size_t size);
  
  inline void set_frequency(float frequency) {
    frequency_ = frequency;
  }
  
  inline void set_structure(float structure) {
    structure_ = structure;
  }
  
  inline void set_brightness(float brightness) {
    brightness_ = brightness;
  }
  
  inline void set_damping(float damping) {
    damping_ = damping;
  }
  
  inline void set_position(float position) {
    position_ = position;
  }
  
  inline void set_resolution(int32_t resolution) {
    resolution -= resolution & 1; // Must be even!
    resolution_ = std::min(resolution, kMaxModes);
  }
  
 private:
  int32_t ComputeFilters();
  float frequency_;
  float structure_;
  float brightness_;
  float position_;
  float previous_position_;
  float damping_;
  
  int32_t resolution_;
  
  stmlib::Svf f_[kMaxModes];
  
  DISALLOW_COPY_AND_ASSIGN(Resonator);
};

}  // namespace rings

#endif  // RINGS_DSP_RESONATOR_H_
