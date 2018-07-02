// Copyright 2013 Olivier Gillet.
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
// Vu-meter

#ifndef ELEMENTS_METER_H_
#define ELEMENTS_METER_H_

#include "stmlib/stmlib.h"

#include "elements/drivers/codec.h"

namespace elements {

class Meter {
 public:
  Meter() { }
  ~Meter() { }

  void Init(int32_t sample_rate) {
    attack_ = 32768 * 100 / sample_rate;
    release_ = 32768 * 4 / sample_rate;
    peak_l_ = 0;
    peak_r_ = 0;
  }

  void Process(Codec::Frame* frames, size_t size) {
    while (size--) {
      int32_t sample;
      int32_t error;
      int32_t coefficient;

      sample = frames->l;
      if (sample < 0) sample = -sample;
      error = sample - peak_l_;
      coefficient = error > 0 ? attack_ : release_;
      peak_l_ += error * coefficient >> 15;

      sample = frames->r;
      if (sample < 0) sample = -sample;
      error = sample - peak_r_;
      coefficient = error > 0 ? attack_ : release_;
      peak_r_ += error * coefficient >> 15;
      ++frames;
    }
  }

  int32_t peak() { return peak_l_ > peak_r_ ? peak_l_ : peak_r_; }

 private:
  int32_t peak_l_;
  int32_t peak_r_;
  int32_t attack_;
  int32_t release_;

  DISALLOW_COPY_AND_ASSIGN(Meter);
};

}  // namespace elements

#endif  // ELEMENTS_METER_H_
