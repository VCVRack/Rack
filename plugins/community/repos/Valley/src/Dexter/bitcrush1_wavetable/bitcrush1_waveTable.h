#ifndef WAVETABLE_BITCRUSH1_H
#define WAVETABLE_BITCRUSH1_H
#define WAVETABLE_BITCRUSH1_NUM 8

#include "bitcrush1_1.h"
#include "bitcrush1_2.h"
#include "bitcrush1_3.h"
#include "bitcrush1_4.h"
#include "bitcrush1_5.h"
#include "bitcrush1_6.h"
#include "bitcrush1_7.h"
#include "bitcrush1_8.h"

static float* wavetable_bitcrush1[WAVETABLE_BITCRUSH1_NUM] = {
    bitcrush1_1_waveTable,
    bitcrush1_2_waveTable,
    bitcrush1_3_waveTable,
    bitcrush1_4_waveTable,
    bitcrush1_5_waveTable,
    bitcrush1_6_waveTable,
    bitcrush1_7_waveTable,
    bitcrush1_8_waveTable
};

static long wavetable_bitcrush1_lengths[WAVETABLE_BITCRUSH1_NUM] = {
    bitcrush1_1_tableLength,
    bitcrush1_2_tableLength,
    bitcrush1_3_tableLength,
    bitcrush1_4_tableLength,
    bitcrush1_5_tableLength,
    bitcrush1_6_tableLength,
    bitcrush1_7_tableLength,
    bitcrush1_8_tableLength
};

#endif // WAVETABLE_BITCRUSH1_H
