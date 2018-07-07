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



namespace tides_parasites {

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
extern const int16_t wav_sine1024[];
extern const int16_t wav_sine128[];
extern const int16_t wav_sine64[];
extern const int16_t wav_sine16[];
extern const int16_t wav_bandlimited_parabola_0[];
extern const int16_t wav_bandlimited_parabola_1[];
extern const int16_t wav_bandlimited_parabola_2[];
extern const int16_t wav_bandlimited_parabola_3[];
extern const int16_t wav_bandlimited_parabola_4[];
extern const int16_t wav_bandlimited_parabola_5[];
extern const int16_t wav_bandlimited_parabola_6[];
extern const int16_t wav_bandlimited_parabola_7[];
extern const int16_t wav_bandlimited_parabola_8[];
extern const int16_t wav_bandlimited_parabola_9[];
extern const int16_t wav_bandlimited_parabola_10[];
extern const int16_t wav_bandlimited_parabola_11[];
extern const int16_t wav_bandlimited_parabola_12[];
extern const int16_t wav_bandlimited_parabola_13[];
extern const int16_t wav_bandlimited_parabola_14[];
extern const int16_t wav_bandlimited_parabola_15[];
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
#define WAV_SINE1024 0
#define WAV_SINE1024_SIZE 1025
#define WAV_SINE128 1
#define WAV_SINE128_SIZE 129
#define WAV_SINE64 2
#define WAV_SINE64_SIZE 65
#define WAV_SINE16 3
#define WAV_SINE16_SIZE 17
#define WAV_BANDLIMITED_PARABOLA_0 4
#define WAV_BANDLIMITED_PARABOLA_0_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_1 5
#define WAV_BANDLIMITED_PARABOLA_1_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_2 6
#define WAV_BANDLIMITED_PARABOLA_2_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_3 7
#define WAV_BANDLIMITED_PARABOLA_3_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_4 8
#define WAV_BANDLIMITED_PARABOLA_4_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_5 9
#define WAV_BANDLIMITED_PARABOLA_5_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_6 10
#define WAV_BANDLIMITED_PARABOLA_6_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_7 11
#define WAV_BANDLIMITED_PARABOLA_7_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_8 12
#define WAV_BANDLIMITED_PARABOLA_8_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_9 13
#define WAV_BANDLIMITED_PARABOLA_9_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_10 14
#define WAV_BANDLIMITED_PARABOLA_10_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_11 15
#define WAV_BANDLIMITED_PARABOLA_11_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_12 16
#define WAV_BANDLIMITED_PARABOLA_12_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_13 17
#define WAV_BANDLIMITED_PARABOLA_13_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_14 18
#define WAV_BANDLIMITED_PARABOLA_14_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_15 19
#define WAV_BANDLIMITED_PARABOLA_15_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_16 20
#define WAV_BANDLIMITED_PARABOLA_16_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_17 21
#define WAV_BANDLIMITED_PARABOLA_17_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_18 22
#define WAV_BANDLIMITED_PARABOLA_18_SIZE 1025
#define WAV_BANDLIMITED_PARABOLA_19 23
#define WAV_BANDLIMITED_PARABOLA_19_SIZE 1025
#define WAV_INVERSE_TAN_AUDIO 24
#define WAV_INVERSE_TAN_AUDIO_SIZE 2049
#define WAV_INVERSE_SIN_AUDIO 25
#define WAV_INVERSE_SIN_AUDIO_SIZE 2049
#define WAV_LINEAR_AUDIO 26
#define WAV_LINEAR_AUDIO_SIZE 2049
#define WAV_SIN_AUDIO 27
#define WAV_SIN_AUDIO_SIZE 2049
#define WAV_TAN_AUDIO 28
#define WAV_TAN_AUDIO_SIZE 2049
#define WAV_REVERSED_CONTROL 29
#define WAV_REVERSED_CONTROL_SIZE 1025
#define WAV_SPIKY_EXP_CONTROL 30
#define WAV_SPIKY_EXP_CONTROL_SIZE 1025
#define WAV_SPIKY_CONTROL 31
#define WAV_SPIKY_CONTROL_SIZE 1025
#define WAV_LINEAR_CONTROL 32
#define WAV_LINEAR_CONTROL_SIZE 1025
#define WAV_BUMP_CONTROL 33
#define WAV_BUMP_CONTROL_SIZE 1025
#define WAV_BUMP_EXP_CONTROL 34
#define WAV_BUMP_EXP_CONTROL_SIZE 1025
#define WAV_NORMAL_CONTROL 35
#define WAV_NORMAL_CONTROL_SIZE 1025
#define WAV_BIPOLAR_FOLD 36
#define WAV_BIPOLAR_FOLD_SIZE 1025
#define WAV_UNIPOLAR_FOLD 37
#define WAV_UNIPOLAR_FOLD_SIZE 1025
#define WT_WAVES 0
#define WT_WAVES_SIZE 45746
#define WS_SMOOTH_BIPOLAR_FOLD 0
#define WS_SMOOTH_BIPOLAR_FOLD_SIZE 1025

}  // namespace tides

#endif  // TIDES_RESOURCES_H_
