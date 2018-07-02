#ifndef WAVETABLE_ADDITIVEBANK_H
#define WAVETABLE_ADDITIVEBANK_H
#define WAVETABLE_ADDITIVEBANK_NUM 8

#include "additiveBank_1.h"
#include "additiveBank_2.h"
#include "additiveBank_3.h"
#include "additiveBank_4.h"
#include "additiveBank_5.h"
#include "additiveBank_6.h"
#include "additiveBank_7.h"
#include "additiveBank_8.h"

static float* wavetable_additiveBank[WAVETABLE_ADDITIVEBANK_NUM] = {
    additiveBank_1_waveTable,
    additiveBank_2_waveTable,
    additiveBank_3_waveTable,
    additiveBank_4_waveTable,
    additiveBank_5_waveTable,
    additiveBank_6_waveTable,
    additiveBank_7_waveTable,
    additiveBank_8_waveTable
};

static long wavetable_additiveBank_lengths[WAVETABLE_ADDITIVEBANK_NUM] = {
    additiveBank_1_tableLength,
    additiveBank_2_tableLength,
    additiveBank_3_tableLength,
    additiveBank_4_tableLength,
    additiveBank_5_tableLength,
    additiveBank_6_tableLength,
    additiveBank_7_tableLength,
    additiveBank_8_tableLength
};

#endif // WAVETABLE_ADDITIVEBANK_H
