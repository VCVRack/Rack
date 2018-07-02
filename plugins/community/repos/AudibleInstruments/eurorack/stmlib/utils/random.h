// Copyright 2012 Olivier Gillet.
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
// Fast 16-bit pseudo random number generator.

#ifndef STMLIB_UTILS_RANDOM_H_
#define STMLIB_UTILS_RANDOM_H_

#include "stmlib/stmlib.h"

namespace stmlib {

class Random {
 public:
  static inline uint32_t state() { return rng_state_; }

  static inline void Seed(uint32_t seed) {
    rng_state_ = seed;
  }

  static inline uint32_t GetWord() {
    rng_state_ = rng_state_ * 1664525L + 1013904223L;
    return state();
  }
  
  static inline int16_t GetSample() {
    return static_cast<int16_t>(GetWord() >> 16);
  }

  static inline float GetFloat() {
    return static_cast<float>(GetWord()) / 4294967296.0f;
  }

 private:
  static uint32_t rng_state_;

  DISALLOW_COPY_AND_ASSIGN(Random);
};

}  // namespace stmlib

#endif  // STMLIB_UTILS_RANDOM_H_
