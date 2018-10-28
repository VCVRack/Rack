// Copyright 2017 Olivier Gillet.
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
// Gate bits are pre-processed to tag edges. A typical gate sequence is:
//
//           ------------
//           
// ----------            ----------
//                                 
// 00000000003111111111114000000000
//  ^        ^   ^       ^
//  |        |   |       |
//  |        |   |      GATE_FLAG_FALLING
//  |        |   |
//  |        |  GATE_FLAG_HIGH
//  |        |
//  |       GATE_FLAG_HIGH | GATE_FLAG_RISING
//  |
// GATE_FLAG_LOW

#ifndef STMLIB_UTILS_GATE_FLAGS_H_
#define STMLIB_UTILS_GATE_FLAGS_H_

#include "stmlib/stmlib.h"

namespace stmlib {

enum GateFlagsBits {
  GATE_FLAG_LOW = 0,
  GATE_FLAG_HIGH = 1,
  GATE_FLAG_RISING = 2,
  GATE_FLAG_FALLING = 4,
};

typedef uint8_t GateFlags;

inline GateFlags ExtractGateFlags(GateFlags previous, bool current) {
  previous &= GATE_FLAG_HIGH;
  if (current) {
    return previous ? GATE_FLAG_HIGH : (GATE_FLAG_RISING | GATE_FLAG_HIGH);
  } else {
    return previous ? GATE_FLAG_FALLING : GATE_FLAG_LOW;
  }
}

}  // namespace stmlib

#endif  // STMLIB_UTILS_GATE_FLAGS_H_
