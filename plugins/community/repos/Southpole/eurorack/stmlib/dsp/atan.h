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
// Fast arc-tangent routines.

#ifndef STMLIB_DSP_ATAN_H_
#define STMLIB_DSP_ATAN_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/rsqrt.h"

#include <cmath>

namespace stmlib {

static inline uint16_t fast_atan2(float y, float x) {
  static const uint32_t sign_mask = 0x80000000;
  static const float b = 0.596227f;
  uint32_t ux_s = sign_mask & unsafe_bit_cast<uint32_t, float>(x);
  uint32_t uy_s = sign_mask & unsafe_bit_cast<uint32_t, float>(y);
  uint32_t offset = ((~ux_s & uy_s) >> 29 | ux_s >> 30) << 14;
  float bxy_a = fabs(b * x * y);
  float num = bxy_a + y * y;
  float atan_1q = num / (x * x + bxy_a + num);
  uint32_t uatan_2q = (ux_s ^ uy_s) | unsafe_bit_cast<uint32_t, float>(atan_1q);
  return unsafe_bit_cast<float, uint32_t>(uatan_2q) * 16384 + offset;
} 

extern const uint16_t atan_lut[513];

static inline uint16_t fast_atan2r(float y, float x, float* r) {
  float squared_magnitude = x * x + y * y;
  if (squared_magnitude == 0.0f) {
    *r = 0.0f;
    return 0.0f;
  }
  float rinv = fast_rsqrt_carmack(squared_magnitude);
  *r = rinv * squared_magnitude;

  static const uint32_t sign_mask = 0x80000000;
  uint32_t ux_s = sign_mask & unsafe_bit_cast<uint32_t, float>(x);
  uint32_t uy_s = sign_mask & unsafe_bit_cast<uint32_t, float>(y);
  uint32_t quadrant = ((~ux_s & uy_s) >> 29 | ux_s >> 30);
  uint16_t angle = 0;
  x = fabs(x);
  y = fabs(y);
  if (y > x) {
    angle = 16384 - atan_lut[static_cast<uint32_t>(x * rinv * 512.0f + 0.5f)];
  } else {
    angle = atan_lut[static_cast<uint32_t>(y * rinv * 512.0f + 0.5f)];
  }
  if (ux_s ^ uy_s) {
    angle = -angle;
  }
  return angle + (quadrant << 14);
}

}  // namespace stmlib

#endif  // STMLIB_DSP_ATAN_H_
