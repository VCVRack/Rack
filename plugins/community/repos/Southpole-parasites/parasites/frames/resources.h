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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef FRAMES_RESOURCES_H_
#define FRAMES_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace frames {

typedef uint8_t ResourceId;

extern const char* string_table[];

extern const uint16_t* lookup_table_table[];

extern const uint32_t* lookup_table_hr_table[];

extern const uint8_t* wt_table[];

extern const uint16_t lut_easing_in_quartic[];
extern const uint16_t lut_easing_out_quartic[];
extern const uint16_t lut_easing_in_out_sine[];
extern const uint16_t lut_easing_in_out_bounce[];
extern const uint16_t lut_response_balance[];
extern const uint16_t lut_vca_linear[];
extern const uint16_t lut_exponential[];
extern const uint32_t lut_increments[];
extern const uint32_t lut_euclidean[];
extern const uint8_t wt_lfo_waveforms[];
#define STR_DUMMY 0  // dummy
#define LUT_EASING_IN_QUARTIC 0
#define LUT_EASING_IN_QUARTIC_SIZE 1025
#define LUT_EASING_OUT_QUARTIC 1
#define LUT_EASING_OUT_QUARTIC_SIZE 1025
#define LUT_EASING_IN_OUT_SINE 2
#define LUT_EASING_IN_OUT_SINE_SIZE 1025
#define LUT_EASING_IN_OUT_BOUNCE 3
#define LUT_EASING_IN_OUT_BOUNCE_SIZE 1025
#define LUT_RESPONSE_BALANCE 4
#define LUT_RESPONSE_BALANCE_SIZE 256
#define LUT_VCA_LINEAR 5
#define LUT_VCA_LINEAR_SIZE 1025
#define LUT_EXPONENTIAL 6
#define LUT_EXPONENTIAL_SIZE 256
#define LUT_INCREMENTS 0
#define LUT_INCREMENTS_SIZE 159
#define LUT_EUCLIDEAN 1
#define LUT_EUCLIDEAN_SIZE 1024
#define WT_LFO_WAVEFORMS 0
#define WT_LFO_WAVEFORMS_SIZE 4626

}  // namespace frames

#endif  // FRAMES_RESOURCES_H_
