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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef STAGES_RESOURCES_H_
#define STAGES_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace stages {

typedef uint8_t ResourceId;

extern const float* lookup_table_table[];

extern const float lut_env_frequency[];
extern const float lut_portamento_coefficient[];
extern const float lut_sine[];
#define LUT_ENV_FREQUENCY 0
#define LUT_ENV_FREQUENCY_SIZE 4096
#define LUT_PORTAMENTO_COEFFICIENT 1
#define LUT_PORTAMENTO_COEFFICIENT_SIZE 512
#define LUT_SINE 2
#define LUT_SINE_SIZE 1281

}  // namespace stages

#endif  // STAGES_RESOURCES_H_
