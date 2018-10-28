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
// Pseudo-random generator used as a fallback when we need more random values
// than available in the hardware RNG buffer.

#ifndef MARBLES_RANDOM_RANDOM_GENERATOR_H_
#define MARBLES_RANDOM_RANDOM_GENERATOR_H_

#include "stmlib/stmlib.h"

#include "stmlib/utils/ring_buffer.h"

namespace marbles {

class RandomGenerator {
 public:
  RandomGenerator() { }
  ~RandomGenerator() { }
  
  inline void Init(uint32_t seed) {
    state_ = seed;
  }
  
  inline void Mix(uint32_t word) {
    // state_ ^= word;
  }
  
  inline uint32_t GetWord() {
    state_ = state_ * 1664525L + 1013904223L;
    return state_;
  }
 
 private:
  uint32_t state_;
  
  DISALLOW_COPY_AND_ASSIGN(RandomGenerator);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_RANDOM_GENERATOR_H_
