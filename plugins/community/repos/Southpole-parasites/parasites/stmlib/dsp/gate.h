// Copyright 2015 Olivier Gillet.
// Copyright 2016 Matthias Puech.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
// Modified by: Matthias Puech (matthias.puech@gmail.com)
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
// Dynamic Gate

#ifndef GATE_H_
#define GATE_H_

#include "stmlib/dsp/dsp.h"

#include "parameters.h"

const float kThresholdLow = 0.002f * 32768.0f;
const float kThresholdHigh = 0.05f * 32768.0f;

class Gate {
 public:
  Gate() { }
  ~Gate() { }

  void Init() {
    peak_ = 0.5f;
  }

  inline void Process(ShortFrame* in_out) {
    size_t size = kBlockSize;
    while (size--) {
      short l = (*in_out).l;
      short r = (*in_out).r;
      SLOPE(peak_, fabs(l), 0.05f, 0.00003f);
      float gain = peak_ < kThresholdLow ? 0.0f :
        peak_ > kThresholdHigh ? 1.0f :
        (peak_ - kThresholdLow) / (kThresholdHigh - kThresholdLow);
      (*in_out).l = l * gain;
      (*in_out).r = r * gain;
      ++in_out;
    }
  }

 private:
  float peak_;

  DISALLOW_COPY_AND_ASSIGN(Gate);
};

#endif
