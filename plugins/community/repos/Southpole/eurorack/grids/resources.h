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


#ifndef GRIDS_RESOURCES_H_
#define GRIDS_RESOURCES_H_


#include "avrlib/base.h"

#include <avr/pgmspace.h>


#include "avrlib/resources_manager.h"

namespace grids {

typedef uint8_t ResourceId;

extern const prog_char* const string_table[];

extern const prog_uint16_t* const lookup_table_table[];

extern const prog_uint32_t* const lookup_table32_table[];

extern const prog_uint8_t* const node_table[];

extern const prog_uint32_t lut_res_euclidean[] PROGMEM;
extern const prog_uint32_t lut_res_tempo_phase_increment[] PROGMEM;
extern const prog_uint8_t node_0[] PROGMEM;
extern const prog_uint8_t node_1[] PROGMEM;
extern const prog_uint8_t node_2[] PROGMEM;
extern const prog_uint8_t node_3[] PROGMEM;
extern const prog_uint8_t node_4[] PROGMEM;
extern const prog_uint8_t node_5[] PROGMEM;
extern const prog_uint8_t node_6[] PROGMEM;
extern const prog_uint8_t node_7[] PROGMEM;
extern const prog_uint8_t node_8[] PROGMEM;
extern const prog_uint8_t node_9[] PROGMEM;
extern const prog_uint8_t node_10[] PROGMEM;
extern const prog_uint8_t node_11[] PROGMEM;
extern const prog_uint8_t node_12[] PROGMEM;
extern const prog_uint8_t node_13[] PROGMEM;
extern const prog_uint8_t node_14[] PROGMEM;
extern const prog_uint8_t node_15[] PROGMEM;
extern const prog_uint8_t node_16[] PROGMEM;
extern const prog_uint8_t node_17[] PROGMEM;
extern const prog_uint8_t node_18[] PROGMEM;
extern const prog_uint8_t node_19[] PROGMEM;
extern const prog_uint8_t node_20[] PROGMEM;
extern const prog_uint8_t node_21[] PROGMEM;
extern const prog_uint8_t node_22[] PROGMEM;
extern const prog_uint8_t node_23[] PROGMEM;
extern const prog_uint8_t node_24[] PROGMEM;
#define STR_RES_DUMMY 0  // dummy
#define LUT_RES_EUCLIDEAN 0
#define LUT_RES_EUCLIDEAN_SIZE 1024
#define LUT_RES_TEMPO_PHASE_INCREMENT 1
#define LUT_RES_TEMPO_PHASE_INCREMENT_SIZE 512
#define NODE_0 0
#define NODE_0_SIZE 96
#define NODE_1 1
#define NODE_1_SIZE 96
#define NODE_2 2
#define NODE_2_SIZE 96
#define NODE_3 3
#define NODE_3_SIZE 96
#define NODE_4 4
#define NODE_4_SIZE 96
#define NODE_5 5
#define NODE_5_SIZE 96
#define NODE_6 6
#define NODE_6_SIZE 96
#define NODE_7 7
#define NODE_7_SIZE 96
#define NODE_8 8
#define NODE_8_SIZE 96
#define NODE_9 9
#define NODE_9_SIZE 96
#define NODE_10 10
#define NODE_10_SIZE 96
#define NODE_11 11
#define NODE_11_SIZE 96
#define NODE_12 12
#define NODE_12_SIZE 96
#define NODE_13 13
#define NODE_13_SIZE 96
#define NODE_14 14
#define NODE_14_SIZE 96
#define NODE_15 15
#define NODE_15_SIZE 96
#define NODE_16 16
#define NODE_16_SIZE 96
#define NODE_17 17
#define NODE_17_SIZE 96
#define NODE_18 18
#define NODE_18_SIZE 96
#define NODE_19 19
#define NODE_19_SIZE 96
#define NODE_20 20
#define NODE_20_SIZE 96
#define NODE_21 21
#define NODE_21_SIZE 96
#define NODE_22 22
#define NODE_22_SIZE 96
#define NODE_23 23
#define NODE_23_SIZE 96
#define NODE_24 24
#define NODE_24_SIZE 96
typedef avrlib::ResourcesManager<
    ResourceId,
    avrlib::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace grids

#endif  // GRIDS_RESOURCES_H_
