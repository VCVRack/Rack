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
// Control of several parameters from one single knob.

#ifndef STREAMS_META_PARAMETERS_H_
#define STREAMS_META_PARAMETERS_H_

#include "stmlib/stmlib.h"

namespace streams {

inline void ComputeAmountOffset(
    int32_t value,
    int32_t* amount,
    int32_t* offset) {
  if (value < 32768) {
    value = 32767 - value;
    value = value * value >> 15;
    *amount = (32767 - value) << 1;
    *offset = 0;
  } else {
    *amount = 65535 - ((value - 32768) << 1);
    *offset = (value - 32768) << 1;
  }
}

inline void ComputeAttackDecay(int32_t shape, uint16_t* a, uint16_t* d) {
  if (shape < 32768) {
    *a = 0;
    *d = 13 * (shape >> 3) + 12288;
  } else if (shape < 49152) {
    *a = (shape - 32768) << 1;
    *d = 65535 - ((shape - 32768) >> 1) * 3;
  } else {
    *a = 32768 - ((shape - 49152) >> 2) * 5;
    *d = 65535 - ((shape - 32768) >> 1) * 3;
  }
}

}  // namespace streams

#endif  // STREAMS_META_PARAMETERS_H_
