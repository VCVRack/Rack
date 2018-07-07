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
// Resources definitions.
//
// Automatically generated with:
// make -f warps/makefile resources


#ifndef WARPS_RESOURCES_H_
#define WARPS_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace warps {

typedef uint8_t ResourceId;

extern const float* filter_bank_table[];

extern const float* lookup_table_table[];

extern const float* wav_table[];

extern const float fb__87_8000[];
extern const float fb_110_8000[];
extern const float fb_139_8000[];
extern const float fb_175_8000[];
extern const float fb_220_8000[];
extern const float fb_277_8000[];
extern const float fb_349_8000[];
extern const float fb_440_8000[];
extern const float fb_554_8000[];
extern const float fb_698_8000[];
extern const float fb_880_8000[];
extern const float fb_1109_8000[];
extern const float fb_1397_8000[];
extern const float fb_1760_32000[];
extern const float fb_2217_32000[];
extern const float fb_2794_32000[];
extern const float fb_3520_32000[];
extern const float fb_4435_32000[];
extern const float fb_5588_32000[];
extern const float fb_7040_96000[];
extern const float lut_sin[];
extern const float lut_arcsin[];
extern const float lut_xfade_in[];
extern const float lut_xfade_out[];
extern const float lut_bipolar_fold[];
extern const float lut_midi_to_f_high[];
extern const float lut_midi_to_f_low[];
extern const float lut_pot_curve[];
extern const float lut_ap_poles[];
extern const float wav_sine_i[];
extern const float wav_sine_q[];
extern const float wav_harmonics_i[];
extern const float wav_harmonics_q[];
extern const float wav_buzzy_i[];
extern const float wav_buzzy_q[];
#define FB__87_8000 0
#define FB__87_8000_SIZE 7
#define FB_110_8000 1
#define FB_110_8000_SIZE 7
#define FB_139_8000 2
#define FB_139_8000_SIZE 7
#define FB_175_8000 3
#define FB_175_8000_SIZE 7
#define FB_220_8000 4
#define FB_220_8000_SIZE 7
#define FB_277_8000 5
#define FB_277_8000_SIZE 7
#define FB_349_8000 6
#define FB_349_8000_SIZE 7
#define FB_440_8000 7
#define FB_440_8000_SIZE 7
#define FB_554_8000 8
#define FB_554_8000_SIZE 7
#define FB_698_8000 9
#define FB_698_8000_SIZE 7
#define FB_880_8000 10
#define FB_880_8000_SIZE 7
#define FB_1109_8000 11
#define FB_1109_8000_SIZE 7
#define FB_1397_8000 12
#define FB_1397_8000_SIZE 7
#define FB_1760_32000 13
#define FB_1760_32000_SIZE 7
#define FB_2217_32000 14
#define FB_2217_32000_SIZE 7
#define FB_2794_32000 15
#define FB_2794_32000_SIZE 7
#define FB_3520_32000 16
#define FB_3520_32000_SIZE 7
#define FB_4435_32000 17
#define FB_4435_32000_SIZE 7
#define FB_5588_32000 18
#define FB_5588_32000_SIZE 7
#define FB_7040_96000 19
#define FB_7040_96000_SIZE 7
#define LUT_SIN 0
#define LUT_SIN_SIZE 1281
#define LUT_ARCSIN 1
#define LUT_ARCSIN_SIZE 257
#define LUT_XFADE_IN 2
#define LUT_XFADE_IN_SIZE 257
#define LUT_XFADE_OUT 3
#define LUT_XFADE_OUT_SIZE 257
#define LUT_BIPOLAR_FOLD 4
#define LUT_BIPOLAR_FOLD_SIZE 4097
#define LUT_MIDI_TO_F_HIGH 5
#define LUT_MIDI_TO_F_HIGH_SIZE 256
#define LUT_MIDI_TO_F_LOW 6
#define LUT_MIDI_TO_F_LOW_SIZE 256
#define LUT_POT_CURVE 7
#define LUT_POT_CURVE_SIZE 513
#define LUT_AP_POLES 8
#define LUT_AP_POLES_SIZE 17
#define WAV_SINE_I 0
#define WAV_SINE_I_SIZE 1025
#define WAV_SINE_Q 1
#define WAV_SINE_Q_SIZE 1025
#define WAV_HARMONICS_I 2
#define WAV_HARMONICS_I_SIZE 1025
#define WAV_HARMONICS_Q 3
#define WAV_HARMONICS_Q_SIZE 1025
#define WAV_BUZZY_I 4
#define WAV_BUZZY_I_SIZE 1025
#define WAV_BUZZY_Q 5
#define WAV_BUZZY_Q_SIZE 1025

}  // namespace warps

#endif  // WARPS_RESOURCES_H_
