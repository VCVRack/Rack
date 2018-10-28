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
// Simple ramp generator.

#ifndef MARBLES_RAMP_RAMP_GENERATOR_H_
#define MARBLES_RAMP_RAMP_GENERATOR_H_

#include "stmlib/stmlib.h"

namespace marbles {

class RampGenerator {
 public:
  RampGenerator() { }
  ~RampGenerator() { }
  
  void Init() {
    phase_ = 0.0f;
  }

  void Render(float frequency, float* out, size_t size) {
    while (size--) {
      phase_ += frequency;
      if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
      }
      *out++ = phase_;
    }
  }
  
 private:
  float phase_;

  DISALLOW_COPY_AND_ASSIGN(RampGenerator);
};

}  // namespace marbles

#endif  // MARBLES_RAMP_RAMP_GENERATOR_H_
