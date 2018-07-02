#ifndef WAVETABLE_ADDITIVE1_H
#define WAVETABLE_ADDITIVE1_H
#define WAVETABLE_ADDITIVE1_NUM 8

#include "additive1_1.h"
#include "additive1_2.h"
#include "additive1_3.h"
#include "additive1_4.h"
#include "additive1_5.h"
#include "additive1_6.h"
#include "additive1_7.h"
#include "additive1_8.h"

static float* wavetable_additive1[WAVETABLE_ADDITIVE1_NUM] = {
    additive1_1_waveTable,
    additive1_2_waveTable,
    additive1_3_waveTable,
    additive1_4_waveTable,
    additive1_5_waveTable,
    additive1_6_waveTable,
    additive1_7_waveTable,
    additive1_8_waveTable
};

static long wavetable_additive1_lengths[WAVETABLE_ADDITIVE1_NUM] = {
    additive1_1_tableLength,
    additive1_2_tableLength,
    additive1_3_tableLength,
    additive1_4_tableLength,
    additive1_5_tableLength,
    additive1_6_tableLength,
    additive1_7_tableLength,
    additive1_8_tableLength
};

#endif // WAVETABLE_ADDITIVE1_H
