#ifndef WAVETABLE_THEREMIN_H
#define WAVETABLE_THEREMIN_H
#define WAVETABLE_THEREMIN_NUM 8

#include "theremin_1.h"
#include "theremin_2.h"
#include "theremin_3.h"
#include "theremin_4.h"
#include "theremin_5.h"
#include "theremin_6.h"
#include "theremin_7.h"
#include "theremin_8.h"

static float* wavetable_theremin[WAVETABLE_THEREMIN_NUM] = {
    theremin_1_waveTable,
    theremin_2_waveTable,
    theremin_3_waveTable,
    theremin_4_waveTable,
    theremin_5_waveTable,
    theremin_6_waveTable,
    theremin_7_waveTable,
    theremin_8_waveTable
};

static long wavetable_theremin_lengths[WAVETABLE_THEREMIN_NUM] = {
    theremin_1_tableLength,
    theremin_2_tableLength,
    theremin_3_tableLength,
    theremin_4_tableLength,
    theremin_5_tableLength,
    theremin_6_tableLength,
    theremin_7_tableLength,
    theremin_8_tableLength
};

#endif // WAVETABLE_THEREMIN_H
