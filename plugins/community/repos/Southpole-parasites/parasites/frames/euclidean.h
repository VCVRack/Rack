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
// Quad euclidean sequencer with envelope generation.

#ifndef FRAMES_EUCLIDEAN_
#define FRAMES_EUCLIDEAN_

#include "stmlib/stmlib.h"

namespace frames {

class Euclidean {
 public:
  Euclidean() { }
  ~Euclidean() { }
  
  void Init();
  void Render();
  void Step(int32_t clock);

  inline uint8_t level() const {
    return value_ >> 8;
  }

  inline uint8_t gate() const {
    return gate_;
  }

  inline const uint16_t dac_code() const {
    return dac_code_;
  }

  inline void set_length(uint8_t length) {
    length_ = length;
  }

  inline void set_fill(uint16_t fill) {
    fill_ = fill;
  }

  inline void set_rotate(uint16_t rotate) {
    rotate_ = rotate;
  }

  inline void set_shape(uint16_t shape) {
    shape_ = shape;
  }

 private:

  void ComputeAttackDecay(uint16_t shape, uint16_t* a, uint16_t* d);

  uint8_t length_;
  uint16_t fill_;
  uint16_t rotate_;
  uint16_t shape_;

  uint32_t value_;
  uint32_t phase_;
  uint16_t attack_;
  bool gate_;
  uint16_t dac_code_;

  DISALLOW_COPY_AND_ASSIGN(Euclidean);
};

}  // namespace frames

#endif  // FRAMES_EUCLIDEAN_
