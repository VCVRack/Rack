#ifndef WAVETABLE_CELLO2_H
#define WAVETABLE_CELLO2_H
#define WAVETABLE_CELLO2_NUM 8

#include "cello2_1.h"
#include "cello2_2.h"
#include "cello2_3.h"
#include "cello2_4.h"
#include "cello2_5.h"
#include "cello2_6.h"
#include "cello2_7.h"
#include "cello2_8.h"

static float* wavetable_cello2[WAVETABLE_CELLO2_NUM] = {
    cello2_1_waveTable,
    cello2_2_waveTable,
    cello2_3_waveTable,
    cello2_4_waveTable,
    cello2_5_waveTable,
    cello2_6_waveTable,
    cello2_7_waveTable,
    cello2_8_waveTable
};

static long wavetable_cello2_lengths[WAVETABLE_CELLO2_NUM] = {
    cello2_1_tableLength,
    cello2_2_tableLength,
    cello2_3_tableLength,
    cello2_4_tableLength,
    cello2_5_tableLength,
    cello2_6_tableLength,
    cello2_7_tableLength,
    cello2_8_tableLength
};

#endif // WAVETABLE_CELLO2_H
