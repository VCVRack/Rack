#ifndef WAVETABLE_TEEEKS_H
#define WAVETABLE_TEEEKS_H
#define WAVETABLE_TEEEKS_NUM 8

#include "TeeEks_1.h"
#include "TeeEks_2.h"
#include "TeeEks_3.h"
#include "TeeEks_4.h"
#include "TeeEks_5.h"
#include "TeeEks_6.h"
#include "TeeEks_7.h"
#include "TeeEks_8.h"

static float* wavetable_TeeEks[WAVETABLE_TEEEKS_NUM] = {
    TeeEks_1_waveTable,
    TeeEks_2_waveTable,
    TeeEks_3_waveTable,
    TeeEks_4_waveTable,
    TeeEks_5_waveTable,
    TeeEks_6_waveTable,
    TeeEks_7_waveTable,
    TeeEks_8_waveTable
};

static long wavetable_TeeEks_lengths[WAVETABLE_TEEEKS_NUM] = {
    TeeEks_1_tableLength,
    TeeEks_2_tableLength,
    TeeEks_3_tableLength,
    TeeEks_4_tableLength,
    TeeEks_5_tableLength,
    TeeEks_6_tableLength,
    TeeEks_7_tableLength,
    TeeEks_8_tableLength
};

#endif // WAVETABLE_TEEEKS_H
