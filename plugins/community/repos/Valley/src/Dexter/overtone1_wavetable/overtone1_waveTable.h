#ifndef WAVETABLE_OVERTONE1_H
#define WAVETABLE_OVERTONE1_H
#define WAVETABLE_OVERTONE1_NUM 8

#include "overtone1_1.h"
#include "overtone1_2.h"
#include "overtone1_3.h"
#include "overtone1_4.h"
#include "overtone1_5.h"
#include "overtone1_6.h"
#include "overtone1_7.h"
#include "overtone1_8.h"

static float* wavetable_overtone1[WAVETABLE_OVERTONE1_NUM] = {
    overtone1_1_waveTable,
    overtone1_2_waveTable,
    overtone1_3_waveTable,
    overtone1_4_waveTable,
    overtone1_5_waveTable,
    overtone1_6_waveTable,
    overtone1_7_waveTable,
    overtone1_8_waveTable
};

static long wavetable_overtone1_lengths[WAVETABLE_OVERTONE1_NUM] = {
    overtone1_1_tableLength,
    overtone1_2_tableLength,
    overtone1_3_tableLength,
    overtone1_4_tableLength,
    overtone1_5_tableLength,
    overtone1_6_tableLength,
    overtone1_7_tableLength,
    overtone1_8_tableLength
};

#endif // WAVETABLE_OVERTONE1_H
