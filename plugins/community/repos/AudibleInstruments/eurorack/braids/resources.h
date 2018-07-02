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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef BRAIDS_RESOURCES_H_
#define BRAIDS_RESOURCES_H_


#include "stmlib/stmlib.h"



namespace braids {

typedef uint8_t ResourceId;

extern const char* string_table[];

extern const uint16_t* lookup_table_table[];

extern const int16_t* lookup_table_signed_table[];

extern const uint32_t* lookup_table_hr_table[];

extern const int16_t* waveform_table[];

extern const int16_t* waveshaper_table[];

extern const uint8_t* wt_table[];

extern const uint16_t* char_table[];

extern const uint16_t lut_resonator_coefficient[];
extern const uint16_t lut_resonator_scale[];
extern const uint16_t lut_svf_cutoff[];
extern const uint16_t lut_svf_damp[];
extern const uint16_t lut_svf_scale[];
extern const uint16_t lut_granular_envelope[];
extern const uint16_t lut_granular_envelope_rate[];
extern const uint16_t lut_bowing_envelope[];
extern const uint16_t lut_bowing_friction[];
extern const uint16_t lut_blowing_envelope[];
extern const uint16_t lut_flute_body_filter[];
extern const uint16_t lut_fm_frequency_quantizer[];
extern const uint16_t lut_vco_detune[];
extern const uint16_t lut_bell[];
extern const uint16_t lut_env_expo[];
extern const int16_t lut_blowing_jet[];
extern const uint32_t lut_oscillator_increments[];
extern const uint32_t lut_oscillator_delays[];
extern const uint32_t lut_env_portamento_increments[];
extern const int16_t wav_formant_sine[];
extern const int16_t wav_formant_square[];
extern const int16_t wav_sine[];
extern const int16_t wav_bandlimited_comb_0[];
extern const int16_t wav_bandlimited_comb_1[];
extern const int16_t wav_bandlimited_comb_2[];
extern const int16_t wav_bandlimited_comb_3[];
extern const int16_t wav_bandlimited_comb_4[];
extern const int16_t wav_bandlimited_comb_5[];
extern const int16_t wav_bandlimited_comb_6[];
extern const int16_t wav_bandlimited_comb_7[];
extern const int16_t wav_bandlimited_comb_8[];
extern const int16_t wav_bandlimited_comb_9[];
extern const int16_t wav_bandlimited_comb_10[];
extern const int16_t wav_bandlimited_comb_11[];
extern const int16_t wav_bandlimited_comb_12[];
extern const int16_t wav_bandlimited_comb_13[];
extern const int16_t wav_bandlimited_comb_14[];
extern const int16_t ws_moderate_overdrive[];
extern const int16_t ws_violent_overdrive[];
extern const int16_t ws_sine_fold[];
extern const int16_t ws_tri_fold[];
extern const uint8_t wt_waves[];
extern const uint8_t wt_map[];
extern const uint8_t wt_code[];
extern const uint16_t chr_characters[];
#define STR_DUMMY 0  // dummy
#define LUT_RESONATOR_COEFFICIENT 0
#define LUT_RESONATOR_COEFFICIENT_SIZE 129
#define LUT_RESONATOR_SCALE 1
#define LUT_RESONATOR_SCALE_SIZE 129
#define LUT_SVF_CUTOFF 2
#define LUT_SVF_CUTOFF_SIZE 257
#define LUT_SVF_DAMP 3
#define LUT_SVF_DAMP_SIZE 257
#define LUT_SVF_SCALE 4
#define LUT_SVF_SCALE_SIZE 257
#define LUT_GRANULAR_ENVELOPE 5
#define LUT_GRANULAR_ENVELOPE_SIZE 513
#define LUT_GRANULAR_ENVELOPE_RATE 6
#define LUT_GRANULAR_ENVELOPE_RATE_SIZE 257
#define LUT_BOWING_ENVELOPE 7
#define LUT_BOWING_ENVELOPE_SIZE 752
#define LUT_BOWING_FRICTION 8
#define LUT_BOWING_FRICTION_SIZE 257
#define LUT_BLOWING_ENVELOPE 9
#define LUT_BLOWING_ENVELOPE_SIZE 392
#define LUT_FLUTE_BODY_FILTER 10
#define LUT_FLUTE_BODY_FILTER_SIZE 128
#define LUT_FM_FREQUENCY_QUANTIZER 11
#define LUT_FM_FREQUENCY_QUANTIZER_SIZE 129
#define LUT_VCO_DETUNE 12
#define LUT_VCO_DETUNE_SIZE 257
#define LUT_BELL 13
#define LUT_BELL_SIZE 257
#define LUT_ENV_EXPO 14
#define LUT_ENV_EXPO_SIZE 257
#define LUT_BLOWING_JET 0
#define LUT_BLOWING_JET_SIZE 257
#define LUT_OSCILLATOR_INCREMENTS 0
#define LUT_OSCILLATOR_INCREMENTS_SIZE 97
#define LUT_OSCILLATOR_DELAYS 1
#define LUT_OSCILLATOR_DELAYS_SIZE 97
#define LUT_ENV_PORTAMENTO_INCREMENTS 2
#define LUT_ENV_PORTAMENTO_INCREMENTS_SIZE 128
#define WAV_FORMANT_SINE 0
#define WAV_FORMANT_SINE_SIZE 256
#define WAV_FORMANT_SQUARE 1
#define WAV_FORMANT_SQUARE_SIZE 256
#define WAV_SINE 2
#define WAV_SINE_SIZE 257
#define WAV_BANDLIMITED_COMB_0 3
#define WAV_BANDLIMITED_COMB_0_SIZE 257
#define WAV_BANDLIMITED_COMB_1 4
#define WAV_BANDLIMITED_COMB_1_SIZE 257
#define WAV_BANDLIMITED_COMB_2 5
#define WAV_BANDLIMITED_COMB_2_SIZE 257
#define WAV_BANDLIMITED_COMB_3 6
#define WAV_BANDLIMITED_COMB_3_SIZE 257
#define WAV_BANDLIMITED_COMB_4 7
#define WAV_BANDLIMITED_COMB_4_SIZE 257
#define WAV_BANDLIMITED_COMB_5 8
#define WAV_BANDLIMITED_COMB_5_SIZE 257
#define WAV_BANDLIMITED_COMB_6 9
#define WAV_BANDLIMITED_COMB_6_SIZE 257
#define WAV_BANDLIMITED_COMB_7 10
#define WAV_BANDLIMITED_COMB_7_SIZE 257
#define WAV_BANDLIMITED_COMB_8 11
#define WAV_BANDLIMITED_COMB_8_SIZE 257
#define WAV_BANDLIMITED_COMB_9 12
#define WAV_BANDLIMITED_COMB_9_SIZE 257
#define WAV_BANDLIMITED_COMB_10 13
#define WAV_BANDLIMITED_COMB_10_SIZE 257
#define WAV_BANDLIMITED_COMB_11 14
#define WAV_BANDLIMITED_COMB_11_SIZE 257
#define WAV_BANDLIMITED_COMB_12 15
#define WAV_BANDLIMITED_COMB_12_SIZE 257
#define WAV_BANDLIMITED_COMB_13 16
#define WAV_BANDLIMITED_COMB_13_SIZE 257
#define WAV_BANDLIMITED_COMB_14 17
#define WAV_BANDLIMITED_COMB_14_SIZE 257
#define WS_MODERATE_OVERDRIVE 0
#define WS_MODERATE_OVERDRIVE_SIZE 257
#define WS_VIOLENT_OVERDRIVE 1
#define WS_VIOLENT_OVERDRIVE_SIZE 257
#define WS_SINE_FOLD 2
#define WS_SINE_FOLD_SIZE 257
#define WS_TRI_FOLD 3
#define WS_TRI_FOLD_SIZE 257
#define WT_WAVES 0
#define WT_WAVES_SIZE 33024
#define WT_MAP 1
#define WT_MAP_SIZE 256
#define WT_CODE 2
#define WT_CODE_SIZE 1064
#define CHR_CHARACTERS 0
#define CHR_CHARACTERS_SIZE 256

}  // namespace braids

#endif  // BRAIDS_RESOURCES_H_
