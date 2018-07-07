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
// Mu-law encoding.

#ifndef CLOUDS_DSP_MU_LAW_H_
#define CLOUDS_DSP_MU_LAW_H_

#include "stmlib/stmlib.h"

namespace clouds {

// inline short MuLaw2Lin(uint8_t u_val) {
//   int16_t t;
//   u_val = ~u_val;
//   t = ((u_val & 0xf) << 3) + 0x84;
//   t <<= ((unsigned)u_val & 0x70) >> 4;
//   return ((u_val & 0x80) ? (0x84 - t) : (t - 0x84));
// }

extern int16_t lut_ulaw[256];

inline short MuLaw2Lin(uint8_t u_val) {
  return lut_ulaw[u_val];
}

inline unsigned char Lin2MuLaw(int16_t pcm_val) {
  int16_t mask;
  int16_t seg;
  uint8_t uval;
  pcm_val = pcm_val >> 2;
  if (pcm_val < 0) {
    pcm_val = -pcm_val;
    mask = 0x7f;
  } else {
    mask = 0xff;
  }
  if (pcm_val > 8159) pcm_val = 8159;
  pcm_val += (0x84 >> 2);

  if (pcm_val <= 0x3f) seg = 0;
  else if (pcm_val <= 0x7f) seg = 1;
  else if (pcm_val <= 0xff) seg = 2;
  else if (pcm_val <= 0x1ff) seg = 3;
  else if (pcm_val <= 0x3ff) seg = 4;
  else if (pcm_val <= 0x7ff) seg = 5;
  else if (pcm_val <= 0xfff) seg = 6;
  else if (pcm_val <= 0x1fff) seg = 7;
  else seg = 8;
  if (seg >= 8)
    return static_cast<uint8_t>(0x7f ^ mask);
  else {
    uval = static_cast<uint8_t>((seg << 4) | ((pcm_val >> (seg + 1)) & 0x0f));
    return (uval ^ mask);
  }
}

}  // namespace clouds

#endif  // CLOUDS_DSP_MU_LAW_H_
