#ifndef WAVETABLE_PLUCK_H
#define WAVETABLE_PLUCK_H
#define WAVETABLE_PLUCK_NUM 9

#include "pluck_1.h"
#include "pluck_2.h"
#include "pluck_3.h"
#include "pluck_4.h"
#include "pluck_5.h"
#include "pluck_6.h"
#include "pluck_7.h"
#include "pluck_8.h"
#include "pluck_9.h"

static float* wavetable_pluck[WAVETABLE_PLUCK_NUM] = {
    pluck_1_waveTable,
    pluck_2_waveTable,
    pluck_3_waveTable,
    pluck_4_waveTable,
    pluck_5_waveTable,
    pluck_6_waveTable,
    pluck_7_waveTable,
    pluck_8_waveTable,
    pluck_9_waveTable
};

static long wavetable_pluck_lengths[WAVETABLE_PLUCK_NUM] = {
    pluck_1_tableLength,
    pluck_2_tableLength,
    pluck_3_tableLength,
    pluck_4_tableLength,
    pluck_5_tableLength,
    pluck_6_tableLength,
    pluck_7_tableLength,
    pluck_8_tableLength,
    pluck_9_tableLength
};

#endif // WAVETABLE_PLUCK_H
