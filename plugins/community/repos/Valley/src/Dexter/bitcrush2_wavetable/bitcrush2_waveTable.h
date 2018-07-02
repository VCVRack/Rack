#ifndef WAVETABLE_BITCRUSH2_H
#define WAVETABLE_BITCRUSH2_H
#define WAVETABLE_BITCRUSH2_NUM 8

#include "bitcrush2_1.h"
#include "bitcrush2_2.h"
#include "bitcrush2_3.h"
#include "bitcrush2_4.h"
#include "bitcrush2_5.h"
#include "bitcrush2_6.h"
#include "bitcrush2_7.h"
#include "bitcrush2_8.h"

static float* wavetable_bitcrush2[WAVETABLE_BITCRUSH2_NUM] = {
    bitcrush2_1_waveTable,
    bitcrush2_2_waveTable,
    bitcrush2_3_waveTable,
    bitcrush2_4_waveTable,
    bitcrush2_5_waveTable,
    bitcrush2_6_waveTable,
    bitcrush2_7_waveTable,
    bitcrush2_8_waveTable
};

static long wavetable_bitcrush2_lengths[WAVETABLE_BITCRUSH2_NUM] = {
    bitcrush2_1_tableLength,
    bitcrush2_2_tableLength,
    bitcrush2_3_tableLength,
    bitcrush2_4_tableLength,
    bitcrush2_5_tableLength,
    bitcrush2_6_tableLength,
    bitcrush2_7_tableLength,
    bitcrush2_8_tableLength
};

#endif // WAVETABLE_BITCRUSH2_H
