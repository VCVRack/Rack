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
// Generates a ramp whose frequency is p/q times the frequency of the input.
// Phase is synchronized.

#ifndef MARBLES_RAMP_RAMP_DIVIDER_H_
#define MARBLES_RAMP_RAMP_DIVIDER_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "marbles/ramp/ramp.h"

namespace marbles {

struct Ratio {
  float to_float() { return static_cast<float>(p) / static_cast<float>(q); }
  int p;
  int q;
  template<int n> void Simplify() {
    while ((p % n) == 0 && (q % n) == 0) {
      p /= n;
      q /= n;
    }
  }
};

class RampDivider {
 public:
  RampDivider() { }
  ~RampDivider() { }
  
  void Init() {
    phase_ = 0.0f;
    train_phase_ = 0.0f;
    max_train_phase_ = 1.0f;
    f_ratio_ = 0.99999f;
    reset_counter_ = 1;
  }

  void Process(Ratio ratio, const float* in, float* out, size_t size) {
    while (size--) {
      float new_phase = *in++;
      float frequency = new_phase - phase_;
      if (frequency < 0.0f) {
        frequency += 1.0f;
        --reset_counter_;
        if (!reset_counter_) {
          train_phase_ = new_phase;
          reset_counter_ = ratio.q;
          f_ratio_ = ratio.to_float() * kMaxRampValue;
          frequency = 0.0f;
          max_train_phase_ = static_cast<float>(ratio.q);
        }
      }
      
      train_phase_ += frequency;
      if (train_phase_ >= max_train_phase_) {
        train_phase_ = max_train_phase_;
      }
      
      float output_phase = train_phase_ * f_ratio_;
      output_phase -= static_cast<float>(static_cast<int>(output_phase));
      *out++ = output_phase;
      phase_ = new_phase;
    }
  }

  
 private:
  float phase_;
  float train_phase_;
  float max_train_phase_;
  float f_ratio_;
  int reset_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(RampDivider);
};

}  // namespace marbles

#endif  // MARBLES_RAMP_RAMP_DIVIDER_H_
