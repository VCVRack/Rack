#ifndef WAVETABLE_CELLO1_H
#define WAVETABLE_CELLO1_H
#define WAVETABLE_CELLO1_NUM 8

#include "cello1_1.h"
#include "cello1_2.h"
#include "cello1_3.h"
#include "cello1_4.h"
#include "cello1_5.h"
#include "cello1_6.h"
#include "cello1_7.h"
#include "cello1_8.h"

static float* wavetable_cello1[WAVETABLE_CELLO1_NUM] = {
    cello1_1_waveTable,
    cello1_2_waveTable,
    cello1_3_waveTable,
    cello1_4_waveTable,
    cello1_5_waveTable,
    cello1_6_waveTable,
    cello1_7_waveTable,
    cello1_8_waveTable
};

static long wavetable_cello1_lengths[WAVETABLE_CELLO1_NUM] = {
    cello1_1_tableLength,
    cello1_2_tableLength,
    cello1_3_tableLength,
    cello1_4_tableLength,
    cello1_5_tableLength,
    cello1_6_tableLength,
    cello1_7_tableLength,
    cello1_8_tableLength
};

#endif // WAVETABLE_CELLO1_H
