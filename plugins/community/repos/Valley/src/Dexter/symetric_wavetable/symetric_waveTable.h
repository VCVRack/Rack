#ifndef WAVETABLE_SYMETRIC_H
#define WAVETABLE_SYMETRIC_H
#define WAVETABLE_SYMETRIC_NUM 14

#include "symetric_1.h"
#include "symetric_2.h"
#include "symetric_3.h"
#include "symetric_4.h"
#include "symetric_5.h"
#include "symetric_6.h"
#include "symetric_7.h"
#include "symetric_8.h"
#include "symetric_9.h"
#include "symetric_10.h"
#include "symetric_11.h"
#include "symetric_12.h"
#include "symetric_13.h"
#include "symetric_14.h"

static float* wavetable_symetric[WAVETABLE_SYMETRIC_NUM] = {
    symetric_1_waveTable,
    symetric_2_waveTable,
    symetric_3_waveTable,
    symetric_4_waveTable,
    symetric_5_waveTable,
    symetric_6_waveTable,
    symetric_7_waveTable,
    symetric_8_waveTable,
    symetric_9_waveTable,
    symetric_10_waveTable,
    symetric_11_waveTable,
    symetric_12_waveTable,
    symetric_13_waveTable,
    symetric_14_waveTable
};

static long wavetable_symetric_lengths[WAVETABLE_SYMETRIC_NUM] = {
    symetric_1_tableLength,
    symetric_2_tableLength,
    symetric_3_tableLength,
    symetric_4_tableLength,
    symetric_5_tableLength,
    symetric_6_tableLength,
    symetric_7_tableLength,
    symetric_8_tableLength,
    symetric_9_tableLength,
    symetric_10_tableLength,
    symetric_11_tableLength,
    symetric_12_tableLength,
    symetric_13_tableLength,
    symetric_14_tableLength
};

#endif // WAVETABLE_SYMETRIC_H
