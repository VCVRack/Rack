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
// Macros for linearly interpolating parameters - used when the modulated signal
// is a sine or triangle - which makes the 4kHz quantization obvious.

#ifndef BRAIDS_PARAMETER_INTERPOLATION_H_
#define BRAIDS_PARAMETER_INTERPOLATION_H_

// Macro for linear interpolation of parameters.
#define BEGIN_INTERPOLATE_PARAMETERS \
  int32_t parameter_0_start = previous_parameter_[0]; \
  int32_t parameter_1_start = previous_parameter_[1]; \
  int32_t parameter_0_delta = parameter_[0] - previous_parameter_[0]; \
  int32_t parameter_1_end = parameter_[1] - previous_parameter_[1]; \
  int32_t parameter_increment = 32767 / size; \
  int32_t parameter_xfade = 0;

#define INTERPOLATE_PARAMETERS \
  parameter_xfade += parameter_increment; \
  int32_t parameter_0 = parameter_0_start + \
      (parameter_0_delta * parameter_xfade >> 15); \
  int32_t parameter_1 = parameter_0_start + \
      (parameter_0_delta * parameter_xfade >> 15);

#define END_INTERPOLATE_PARAMETERS \
  previous_parameter_[0] = parameter_[0]; \
  previous_parameter_[1] = parameter_[1];


#define BEGIN_INTERPOLATE_PARAMETER_0 \
  int32_t parameter_0_start = previous_parameter_[0]; \
  int32_t parameter_0_delta = parameter_[0] - previous_parameter_[0]; \
  int32_t parameter_increment = 32767 / size; \
  int32_t parameter_xfade = 0;

#define INTERPOLATE_PARAMETER_0 \
  parameter_xfade += parameter_increment; \
  int32_t parameter_0 = parameter_0_start + \
      (parameter_0_delta * parameter_xfade >> 15);

#define END_INTERPOLATE_PARAMETER_0 \
  previous_parameter_[0] = parameter_[0];


#define BEGIN_INTERPOLATE_PARAMETER_1 \
  int32_t parameter_1_start = previous_parameter_[1]; \
  int32_t parameter_1_delta = parameter_[1] - previous_parameter_[1]; \
  int32_t parameter_increment = 32767 / size; \
  int32_t parameter_xfade = 0;

#define INTERPOLATE_PARAMETER_1 \
  parameter_xfade += parameter_increment; \
  int32_t parameter_1 = parameter_1_start + \
      (parameter_1_delta * parameter_xfade >> 15);

#define END_INTERPOLATE_PARAMETER_1 \
  previous_parameter_[1] = parameter_[1];


#define BEGIN_INTERPOLATE_PARAMETER \
  int32_t parameter_start = previous_parameter_; \
  int32_t parameter_delta = parameter_ - previous_parameter_; \
  int32_t parameter_increment = 32767 / size; \
  int32_t parameter_xfade = 0;

#define INTERPOLATE_PARAMETER \
  parameter_xfade += parameter_increment; \
  int32_t parameter = parameter_start + \
      (parameter_delta * parameter_xfade >> 15);

#define END_INTERPOLATE_PARAMETER \
  previous_parameter_ = parameter_;

#define BEGIN_INTERPOLATE_PHASE_INCREMENT \
  uint32_t phase_increment = previous_phase_increment_; \
  uint32_t phase_increment_increment = \
      previous_phase_increment_ < phase_increment_ \
      ? (phase_increment_ - previous_phase_increment_) / size \
      : ~((previous_phase_increment_ - phase_increment_) / size);

#define INTERPOLATE_PHASE_INCREMENT \
  phase_increment += phase_increment_increment;
  
#define END_INTERPOLATE_PHASE_INCREMENT \
  previous_phase_increment_ = phase_increment;

#endif // BRAIDS_PARAMETER_INTERPOLATION_H_
