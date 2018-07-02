#ifndef WAVETABLE_ADDITIVESAW_H
#define WAVETABLE_ADDITIVESAW_H
#define WAVETABLE_ADDITIVESAW_NUM 16

#include "additiveSaw_1.h"
#include "additiveSaw_2.h"
#include "additiveSaw_3.h"
#include "additiveSaw_4.h"
#include "additiveSaw_5.h"
#include "additiveSaw_6.h"
#include "additiveSaw_7.h"
#include "additiveSaw_8.h"
#include "additiveSaw_9.h"
#include "additiveSaw_10.h"
#include "additiveSaw_11.h"
#include "additiveSaw_12.h"
#include "additiveSaw_13.h"
#include "additiveSaw_14.h"
#include "additiveSaw_15.h"
#include "additiveSaw_16.h"

static float* wavetable_additiveSaw[WAVETABLE_ADDITIVESAW_NUM] = {
    additiveSaw_1_waveTable,
    additiveSaw_2_waveTable,
    additiveSaw_3_waveTable,
    additiveSaw_4_waveTable,
    additiveSaw_5_waveTable,
    additiveSaw_6_waveTable,
    additiveSaw_7_waveTable,
    additiveSaw_8_waveTable,
    additiveSaw_9_waveTable,
    additiveSaw_10_waveTable,
    additiveSaw_11_waveTable,
    additiveSaw_12_waveTable,
    additiveSaw_13_waveTable,
    additiveSaw_14_waveTable,
    additiveSaw_15_waveTable,
    additiveSaw_16_waveTable
};

static long wavetable_additiveSaw_lengths[WAVETABLE_ADDITIVESAW_NUM] = {
    additiveSaw_1_tableLength,
    additiveSaw_2_tableLength,
    additiveSaw_3_tableLength,
    additiveSaw_4_tableLength,
    additiveSaw_5_tableLength,
    additiveSaw_6_tableLength,
    additiveSaw_7_tableLength,
    additiveSaw_8_tableLength,
    additiveSaw_9_tableLength,
    additiveSaw_10_tableLength,
    additiveSaw_11_tableLength,
    additiveSaw_12_tableLength,
    additiveSaw_13_tableLength,
    additiveSaw_14_tableLength,
    additiveSaw_15_tableLength,
    additiveSaw_16_tableLength
};

#endif // WAVETABLE_ADDITIVESAW_H
