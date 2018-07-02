#ifndef WAVETABLE_OVERTONE2_H
#define WAVETABLE_OVERTONE2_H
#define WAVETABLE_OVERTONE2_NUM 8

#include "overtone2_1.h"
#include "overtone2_2.h"
#include "overtone2_3.h"
#include "overtone2_4.h"
#include "overtone2_5.h"
#include "overtone2_6.h"
#include "overtone2_7.h"
#include "overtone2_8.h"

static float* wavetable_overtone2[WAVETABLE_OVERTONE2_NUM] = {
    overtone2_1_waveTable,
    overtone2_2_waveTable,
    overtone2_3_waveTable,
    overtone2_4_waveTable,
    overtone2_5_waveTable,
    overtone2_6_waveTable,
    overtone2_7_waveTable,
    overtone2_8_waveTable
};

static long wavetable_overtone2_lengths[WAVETABLE_OVERTONE2_NUM] = {
    overtone2_1_tableLength,
    overtone2_2_tableLength,
    overtone2_3_tableLength,
    overtone2_4_tableLength,
    overtone2_5_tableLength,
    overtone2_6_tableLength,
    overtone2_7_tableLength,
    overtone2_8_tableLength
};

#endif // WAVETABLE_OVERTONE2_H
