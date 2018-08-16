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
// Cosine oscillator. Generates a cosine between 0.0 and 1.0 with minimal
// CPU use.

#ifndef STMLIB_DSP_COSINE_OSCILLATOR_H_
#define STMLIB_DSP_COSINE_OSCILLATOR_H_

#include "stmlib/stmlib.h"

#include <cmath>

namespace stmlib {

enum CosineOscillatorMode {
  COSINE_OSCILLATOR_APPROXIMATE,
  COSINE_OSCILLATOR_EXACT
};

class CosineOscillator {
 public:
  CosineOscillator() { }
  ~CosineOscillator() { }

  template<CosineOscillatorMode mode>
  inline void Init(float frequency) {
    if (mode == COSINE_OSCILLATOR_APPROXIMATE) {
      InitApproximate(frequency);
    } else {
      iir_coefficient_ = 2.0f * cosf(2.0f * M_PI * frequency);
      initial_amplitude_ = iir_coefficient_ * 0.25f;
    }
    Start();
  }
  
  inline void InitApproximate(float frequency) {
    float sign = 16.0f;
    frequency -= 0.25f;
    if (frequency < 0.0f) {
      frequency = -frequency;
    } else {
      if (frequency > 0.5f) {
        frequency -= 0.5f;
      } else {
        sign = -16.0f;
      }
    }
    iir_coefficient_ = sign * frequency * (1.0f - 2.0f * frequency);
    initial_amplitude_ = iir_coefficient_ * 0.25f;
  }
  
  inline void Start() {
    y1_ = initial_amplitude_;
    y0_ = 0.5f;
  }
  
  inline float value() const {
    return y1_ + 0.5f;
  }

  inline float Next() {
    float temp = y0_;
    y0_ = iir_coefficient_ * y0_ - y1_;
    y1_ = temp;
    return temp + 0.5f;
  }
  
 private:
  float y1_;
  float y0_;
  float iir_coefficient_;
  float initial_amplitude_;

  DISALLOW_COPY_AND_ASSIGN(CosineOscillator);
};

}  // namespace stmlib

#endif  // STMLIB_DSP_COSINE_OSCILLATOR_H_
