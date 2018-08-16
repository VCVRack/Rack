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


#ifndef TIDES_RESOURCES_H_
#define TIDES_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace tides {

typedef uint8_t ResourceId;

extern const uint16_t* lookup_table_table[];

extern const uint32_t* lookup_table_32_table[];

extern const int16_t* waveform_table[];

extern const int16_t* wavetable_table[];

extern const int16_t* waveshaper_table[];

extern const uint16_t lut_attenuverter_curve[];
extern const uint16_t lut_slope_compression[];
extern const uint32_t lut_increments[];
extern const uint32_t lut_cutoff[];
extern const int16_t wav_inverse_tan_audio[];
extern const int16_t wav_inverse_sin_audio[];
extern const int16_t wav_linear_audio[];
extern const int16_t wav_sin_audio[];
extern const int16_t wav_tan_audio[];
extern const int16_t wav_reversed_control[];
extern const int16_t wav_spiky_exp_control[];
extern const int16_t wav_spiky_control[];
extern const int16_t wav_linear_control[];
extern const int16_t wav_bump_control[];
extern const int16_t wav_bump_exp_control[];
extern const int16_t wav_normal_control[];
extern const int16_t wav_bipolar_fold[];
extern const int16_t wav_unipolar_fold[];
extern const int16_t wt_waves[];
extern const int16_t ws_smooth_bipolar_fold[] IN_RAM;
#define LUT_ATTENUVERTER_CURVE 0
#define LUT_ATTENUVERTER_CURVE_SIZE 257
#define LUT_SLOPE_COMPRESSION 1
#define LUT_SLOPE_COMPRESSION_SIZE 257
#define LUT_INCREMENTS 0
#define LUT_INCREMENTS_SIZE 97
#define LUT_CUTOFF 1
#define LUT_CUTOFF_SIZE 513
#define WAV_INVERSE_TAN_AUDIO 0
#define WAV_INVERSE_TAN_AUDIO_SIZE 2049
#define WAV_INVERSE_SIN_AUDIO 1
#define WAV_INVERSE_SIN_AUDIO_SIZE 2049
#define WAV_LINEAR_AUDIO 2
#define WAV_LINEAR_AUDIO_SIZE 2049
#define WAV_SIN_AUDIO 3
#define WAV_SIN_AUDIO_SIZE 2049
#define WAV_TAN_AUDIO 4
#define WAV_TAN_AUDIO_SIZE 2049
#define WAV_REVERSED_CONTROL 5
#define WAV_REVERSED_CONTROL_SIZE 2049
#define WAV_SPIKY_EXP_CONTROL 6
#define WAV_SPIKY_EXP_CONTROL_SIZE 2049
#define WAV_SPIKY_CONTROL 7
#define WAV_SPIKY_CONTROL_SIZE 2049
#define WAV_LINEAR_CONTROL 8
#define WAV_LINEAR_CONTROL_SIZE 2049
#define WAV_BUMP_CONTROL 9
#define WAV_BUMP_CONTROL_SIZE 2049
#define WAV_BUMP_EXP_CONTROL 10
#define WAV_BUMP_EXP_CONTROL_SIZE 2049
#define WAV_NORMAL_CONTROL 11
#define WAV_NORMAL_CONTROL_SIZE 2049
#define WAV_BIPOLAR_FOLD 12
#define WAV_BIPOLAR_FOLD_SIZE 1025
#define WAV_UNIPOLAR_FOLD 13
#define WAV_UNIPOLAR_FOLD_SIZE 1025
#define WT_WAVES 0
#define WT_WAVES_SIZE 45746
#define WS_SMOOTH_BIPOLAR_FOLD 0
#define WS_SMOOTH_BIPOLAR_FOLD_SIZE 1025

}  // namespace tides

#endif  // TIDES_RESOURCES_H_
