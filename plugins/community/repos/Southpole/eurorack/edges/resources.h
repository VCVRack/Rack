// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (olivier@mutable-instruments.net)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef EDGES_RESOURCES_H_
#define EDGES_RESOURCES_H_


#include "avrlibx/avrlibx.h"

#include <avr/pgmspace.h>


#include "avrlibx/resources/resources_manager.h"

namespace edges {

typedef uint8_t ResourceId;

extern const prog_char* const string_table[];

extern const prog_uint16_t* const lookup_table_table[];

extern const prog_uint32_t* const lookup_table_32_table[];

extern const prog_uint8_t* const waveform_table[];

extern const prog_uint16_t lut_res_timer_count[] PROGMEM;
extern const prog_uint16_t lut_res_oscillator_increments[] PROGMEM;
extern const prog_uint16_t lut_res_bitcrusher_increments[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_1[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_2[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_5[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_6[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_1[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_2[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_nes_triangle_5[] PROGMEM;
#define STR_RES_DUMMY 0  // dummy
#define LUT_RES_TIMER_COUNT 0
#define LUT_RES_TIMER_COUNT_SIZE 97
#define LUT_RES_OSCILLATOR_INCREMENTS 1
#define LUT_RES_OSCILLATOR_INCREMENTS_SIZE 97
#define LUT_RES_BITCRUSHER_INCREMENTS 2
#define LUT_RES_BITCRUSHER_INCREMENTS_SIZE 256
#define WAV_RES_BANDLIMITED_TRIANGLE_0 0
#define WAV_RES_BANDLIMITED_TRIANGLE_0_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_1 1
#define WAV_RES_BANDLIMITED_TRIANGLE_1_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_2 2
#define WAV_RES_BANDLIMITED_TRIANGLE_2_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_3 3
#define WAV_RES_BANDLIMITED_TRIANGLE_3_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_4 4
#define WAV_RES_BANDLIMITED_TRIANGLE_4_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_5 5
#define WAV_RES_BANDLIMITED_TRIANGLE_5_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_6 6
#define WAV_RES_BANDLIMITED_TRIANGLE_6_SIZE 513
#define WAV_RES_BANDLIMITED_TRIANGLE_7 7
#define WAV_RES_BANDLIMITED_TRIANGLE_7_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_0 8
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_0_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_1 9
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_1_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_2 10
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_2_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_3 11
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_3_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_4 12
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_4_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_5 13
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_5_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_6 14
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_6_SIZE 513
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_7 15
#define WAV_RES_BANDLIMITED_NES_TRIANGLE_7_SIZE 513
typedef avrlibx::ResourcesManager<
    ResourceId,
    avrlibx::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace edges

#endif  // EDGES_RESOURCES_H_
