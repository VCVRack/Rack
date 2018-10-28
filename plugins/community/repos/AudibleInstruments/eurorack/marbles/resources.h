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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef MARBLES_RESOURCES_H_
#define MARBLES_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace marbles {

typedef uint8_t ResourceId;

extern const float* lookup_table_table[];

extern const float* distributions_table[];

extern const float lut_raised_cosine[];
extern const float lut_sine[];
extern const float lut_logit[];
extern const float dist_icdf_0_0[];
extern const float dist_icdf_0_1[];
extern const float dist_icdf_0_2[];
extern const float dist_icdf_0_3[];
extern const float dist_icdf_0_4[];
extern const float dist_icdf_0_5[];
extern const float dist_icdf_0_6[];
extern const float dist_icdf_0_7[];
extern const float dist_icdf_0_8[];
extern const float dist_icdf_1_0[];
extern const float dist_icdf_1_1[];
extern const float dist_icdf_1_2[];
extern const float dist_icdf_1_3[];
extern const float dist_icdf_1_4[];
extern const float dist_icdf_1_5[];
extern const float dist_icdf_1_6[];
extern const float dist_icdf_1_7[];
extern const float dist_icdf_1_8[];
extern const float dist_icdf_2_0[];
extern const float dist_icdf_2_1[];
extern const float dist_icdf_2_2[];
extern const float dist_icdf_2_3[];
extern const float dist_icdf_2_4[];
extern const float dist_icdf_2_5[];
extern const float dist_icdf_2_6[];
extern const float dist_icdf_2_7[];
extern const float dist_icdf_2_8[];
extern const float dist_icdf_3_0[];
extern const float dist_icdf_3_1[];
extern const float dist_icdf_3_2[];
extern const float dist_icdf_3_3[];
extern const float dist_icdf_3_4[];
extern const float dist_icdf_3_5[];
extern const float dist_icdf_3_6[];
extern const float dist_icdf_3_7[];
extern const float dist_icdf_3_8[];
extern const float dist_icdf_4_0[];
extern const float dist_icdf_4_1[];
extern const float dist_icdf_4_2[];
extern const float dist_icdf_4_3[];
extern const float dist_icdf_4_4[];
extern const float dist_icdf_4_5[];
extern const float dist_icdf_4_6[];
extern const float dist_icdf_4_7[];
extern const float dist_icdf_4_8[];
#define LUT_RAISED_COSINE 0
#define LUT_RAISED_COSINE_SIZE 257
#define LUT_SINE 1
#define LUT_SINE_SIZE 257
#define LUT_LOGIT 2
#define LUT_LOGIT_SIZE 257
#define DIST_ICDF_0_0 0
#define DIST_ICDF_0_0_SIZE 387
#define DIST_ICDF_0_1 1
#define DIST_ICDF_0_1_SIZE 387
#define DIST_ICDF_0_2 2
#define DIST_ICDF_0_2_SIZE 387
#define DIST_ICDF_0_3 3
#define DIST_ICDF_0_3_SIZE 387
#define DIST_ICDF_0_4 4
#define DIST_ICDF_0_4_SIZE 387
#define DIST_ICDF_0_5 5
#define DIST_ICDF_0_5_SIZE 387
#define DIST_ICDF_0_6 6
#define DIST_ICDF_0_6_SIZE 387
#define DIST_ICDF_0_7 7
#define DIST_ICDF_0_7_SIZE 387
#define DIST_ICDF_0_8 8
#define DIST_ICDF_0_8_SIZE 387
#define DIST_ICDF_0_8_GUARD 9
#define DIST_ICDF_0_8_GUARD_SIZE 387
#define DIST_ICDF_1_0 10
#define DIST_ICDF_1_0_SIZE 387
#define DIST_ICDF_1_1 11
#define DIST_ICDF_1_1_SIZE 387
#define DIST_ICDF_1_2 12
#define DIST_ICDF_1_2_SIZE 387
#define DIST_ICDF_1_3 13
#define DIST_ICDF_1_3_SIZE 387
#define DIST_ICDF_1_4 14
#define DIST_ICDF_1_4_SIZE 387
#define DIST_ICDF_1_5 15
#define DIST_ICDF_1_5_SIZE 387
#define DIST_ICDF_1_6 16
#define DIST_ICDF_1_6_SIZE 387
#define DIST_ICDF_1_7 17
#define DIST_ICDF_1_7_SIZE 387
#define DIST_ICDF_1_8 18
#define DIST_ICDF_1_8_SIZE 387
#define DIST_ICDF_1_8_GUARD 19
#define DIST_ICDF_1_8_GUARD_SIZE 387
#define DIST_ICDF_2_0 20
#define DIST_ICDF_2_0_SIZE 387
#define DIST_ICDF_2_1 21
#define DIST_ICDF_2_1_SIZE 387
#define DIST_ICDF_2_2 22
#define DIST_ICDF_2_2_SIZE 387
#define DIST_ICDF_2_3 23
#define DIST_ICDF_2_3_SIZE 387
#define DIST_ICDF_2_4 24
#define DIST_ICDF_2_4_SIZE 387
#define DIST_ICDF_2_5 25
#define DIST_ICDF_2_5_SIZE 387
#define DIST_ICDF_2_6 26
#define DIST_ICDF_2_6_SIZE 387
#define DIST_ICDF_2_7 27
#define DIST_ICDF_2_7_SIZE 387
#define DIST_ICDF_2_8 28
#define DIST_ICDF_2_8_SIZE 387
#define DIST_ICDF_2_8_GUARD 29
#define DIST_ICDF_2_8_GUARD_SIZE 387
#define DIST_ICDF_3_0 30
#define DIST_ICDF_3_0_SIZE 387
#define DIST_ICDF_3_1 31
#define DIST_ICDF_3_1_SIZE 387
#define DIST_ICDF_3_2 32
#define DIST_ICDF_3_2_SIZE 387
#define DIST_ICDF_3_3 33
#define DIST_ICDF_3_3_SIZE 387
#define DIST_ICDF_3_4 34
#define DIST_ICDF_3_4_SIZE 387
#define DIST_ICDF_3_5 35
#define DIST_ICDF_3_5_SIZE 387
#define DIST_ICDF_3_6 36
#define DIST_ICDF_3_6_SIZE 387
#define DIST_ICDF_3_7 37
#define DIST_ICDF_3_7_SIZE 387
#define DIST_ICDF_3_8 38
#define DIST_ICDF_3_8_SIZE 387
#define DIST_ICDF_3_8_GUARD 39
#define DIST_ICDF_3_8_GUARD_SIZE 387
#define DIST_ICDF_4_0 40
#define DIST_ICDF_4_0_SIZE 387
#define DIST_ICDF_4_1 41
#define DIST_ICDF_4_1_SIZE 387
#define DIST_ICDF_4_2 42
#define DIST_ICDF_4_2_SIZE 387
#define DIST_ICDF_4_3 43
#define DIST_ICDF_4_3_SIZE 387
#define DIST_ICDF_4_4 44
#define DIST_ICDF_4_4_SIZE 387
#define DIST_ICDF_4_5 45
#define DIST_ICDF_4_5_SIZE 387
#define DIST_ICDF_4_6 46
#define DIST_ICDF_4_6_SIZE 387
#define DIST_ICDF_4_7 47
#define DIST_ICDF_4_7_SIZE 387
#define DIST_ICDF_4_8 48
#define DIST_ICDF_4_8_SIZE 387
#define DIST_ICDF_4_8_GUARD 49
#define DIST_ICDF_4_8_GUARD_SIZE 387
#define DIST_ICDF_4_0_GUARD 50
#define DIST_ICDF_4_0_GUARD_SIZE 387
#define DIST_ICDF_4_1_GUARD 51
#define DIST_ICDF_4_1_GUARD_SIZE 387
#define DIST_ICDF_4_2_GUARD 52
#define DIST_ICDF_4_2_GUARD_SIZE 387
#define DIST_ICDF_4_3_GUARD 53
#define DIST_ICDF_4_3_GUARD_SIZE 387
#define DIST_ICDF_4_4_GUARD 54
#define DIST_ICDF_4_4_GUARD_SIZE 387
#define DIST_ICDF_4_5_GUARD 55
#define DIST_ICDF_4_5_GUARD_SIZE 387
#define DIST_ICDF_4_6_GUARD 56
#define DIST_ICDF_4_6_GUARD_SIZE 387
#define DIST_ICDF_4_7_GUARD 57
#define DIST_ICDF_4_7_GUARD_SIZE 387
#define DIST__ICDF_4_8_GUARD 58
#define DIST__ICDF_4_8_GUARD_SIZE 387
#define DIST_ICDF_4_8_GUARD_GUARD 59
#define DIST_ICDF_4_8_GUARD_GUARD_SIZE 387

}  // namespace marbles

#endif  // MARBLES_RESOURCES_H_
