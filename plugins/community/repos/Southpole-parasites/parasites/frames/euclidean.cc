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
// Poly LFO.

#include "frames/euclidean.h"

#include <cstdio>
#include <algorithm>

#include "stmlib/utils/random.h"

#include "frames/resources.h"
#include "frames/keyframer.h"

namespace frames {

using namespace std;
using namespace stmlib;

void Euclidean::Init() {
  value_ = 0;
  dac_code_ = Keyframer::ConvertToDacCode(value_, 0);
  length_ = 16;
  fill_ = 0;
  rotate_ = 0;
  shape_ = 4000;
}

void Euclidean::Step(int32_t clock) {
  uint8_t fill = length_ * fill_ / 61000;
  uint8_t rotate = length_ * rotate_ / 65000;
  CONSTRAIN(fill, 0, length_ + 1);
  uint32_t mask = 1 << ((clock + rotate) % length_);
  uint16_t offset = static_cast<uint16_t>(length_) * 32;
  uint32_t pattern = lut_euclidean[offset + fill];

  if (mask & pattern) {
    // we restart the envelope from its current point
    phase_ = attack_ * static_cast<uint32_t>(value_) / 65535;
    gate_ = true;
  } else {
    gate_ = false;
  }
}

inline void Euclidean::ComputeAttackDecay(uint16_t shape, uint16_t* a, uint16_t* d) {
  if (shape < 32768) {
    *a = 0;
    *d = 15 * (shape >> 3) + 1024; // 1024..62868
  } else if (shape < 49152) {
    *a = (shape - 32768) << 1;	// 2..32766
    *d = 65535 - ((shape - 32768) >> 4) * 31; // 65535..33791
  } else {
    *a = 32768 - ((shape - 49152) >> 2) * 7; // 32768..4117
    *d = 65535 - ((shape - 32768) >> 4) * 31; // 33791..2048
  }
}

void Euclidean::Render() {
  uint16_t decay;
  ComputeAttackDecay(shape_, &attack_, &decay);
  attack_ /= 3;
  decay /= 2;

  if (phase_ < attack_) {
    value_ = phase_ * 65535 / attack_;
  } else if (phase_ < attack_ + decay) {
    value_ = 65535 - 65535 * (phase_ - attack_) / decay;
  } else {
    value_ = 0;
  }

  uint16_t val = value_ * value_ / 65535 * value_ / 65535;

  phase_++;
  dac_code_ = Keyframer::ConvertToDacCode(val, 0);
}

}  // namespace frames
