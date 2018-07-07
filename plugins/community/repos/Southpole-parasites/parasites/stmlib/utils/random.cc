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
// Random number generator.

#include "stmlib/utils/random.h"

namespace stmlib {

/* static */
uint32_t Random::rng_state_ = 0x21;

/* input x is a 0.16 fixed-point number in [0,1)
   function returns -log2(x) as a 4.16 fixed-point number in [0, 16)
*/
uint32_t Random::nlog2_16 (uint16_t x) {
    uint32_t r = 0;
    uint32_t t, a = x;

    if ((t = (a *   256)      ) < 0x10000) { a = t; r += 0x80000; }
    if ((t = (a *    16)      ) < 0x10000) { a = t; r += 0x40000; }
    if ((t = (a *     4)      ) < 0x10000) { a = t; r += 0x20000; }
    if ((t = (a *     2)      ) < 0x10000) { a = t; r += 0x10000; }
    if ((t = (a *     3) >>  1) < 0x10000) { a = t; r += 0x095c0; }
    if ((t = (a *     5) >>  2) < 0x10000) { a = t; r += 0x0526a; }
    if ((t = (a *     9) >>  3) < 0x10000) { a = t; r += 0x02b80; }
    if ((t = (a *    17) >>  4) < 0x10000) { a = t; r += 0x01664; }
    if ((t = (a *    33) >>  5) < 0x10000) { a = t; r += 0x00b5d; }
    if ((t = (a *    65) >>  6) < 0x10000) { a = t; r += 0x005ba; }
    if ((t = (a *   129) >>  7) < 0x10000) { a = t; r += 0x002e0; }
    if ((t = (a *   257) >>  8) < 0x10000) { a = t; r += 0x00171; }
    if ((t = (a *   513) >>  9) < 0x10000) { a = t; r += 0x000b8; }
    if ((t = (a *  1025) >> 10) < 0x10000) { a = t; r += 0x0005c; }
    if ((t = (a *  2049) >> 11) < 0x10000) { a = t; r += 0x0002e; }
    if ((t = (a *  4097) >> 12) < 0x10000) { a = t; r += 0x00017; }
    if ((t = (a *  8193) >> 13) < 0x10000) { a = t; r += 0x0000c; }
    if ((t = (a * 16385) >> 14) < 0x10000) { a = t; r += 0x00006; }
    if ((t = (a * 32769) >> 15) < 0x10000) { a = t; r += 0x00003; }
    if ((t = (a * 65537) >> 16) < 0x10000) { a = t; r += 0x00001; }
    if (r == 0) r++;  // don't let log be zero, since we'll divide by it
    return r;
}

}  // namespace stmlib
