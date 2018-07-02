#ifndef WAVETABLE_ADDITIVESINE_H
#define WAVETABLE_ADDITIVESINE_H
#define WAVETABLE_ADDITIVESINE_NUM 16

#include "additiveSine_1.h"
#include "additiveSine_2.h"
#include "additiveSine_3.h"
#include "additiveSine_4.h"
#include "additiveSine_5.h"
#include "additiveSine_6.h"
#include "additiveSine_7.h"
#include "additiveSine_8.h"
#include "additiveSine_9.h"
#include "additiveSine_10.h"
#include "additiveSine_11.h"
#include "additiveSine_12.h"
#include "additiveSine_13.h"
#include "additiveSine_14.h"
#include "additiveSine_15.h"
#include "additiveSine_16.h"

static float* wavetable_additiveSine[WAVETABLE_ADDITIVESINE_NUM] = {
    additiveSine_1_waveTable,
    additiveSine_2_waveTable,
    additiveSine_3_waveTable,
    additiveSine_4_waveTable,
    additiveSine_5_waveTable,
    additiveSine_6_waveTable,
    additiveSine_7_waveTable,
    additiveSine_8_waveTable,
    additiveSine_9_waveTable,
    additiveSine_10_waveTable,
    additiveSine_11_waveTable,
    additiveSine_12_waveTable,
    additiveSine_13_waveTable,
    additiveSine_14_waveTable,
    additiveSine_15_waveTable,
    additiveSine_16_waveTable
};

static long wavetable_additiveSine_lengths[WAVETABLE_ADDITIVESINE_NUM] = {
    additiveSine_1_tableLength,
    additiveSine_2_tableLength,
    additiveSine_3_tableLength,
    additiveSine_4_tableLength,
    additiveSine_5_tableLength,
    additiveSine_6_tableLength,
    additiveSine_7_tableLength,
    additiveSine_8_tableLength,
    additiveSine_9_tableLength,
    additiveSine_10_tableLength,
    additiveSine_11_tableLength,
    additiveSine_12_tableLength,
    additiveSine_13_tableLength,
    additiveSine_14_tableLength,
    additiveSine_15_tableLength,
    additiveSine_16_tableLength
};

#endif // WAVETABLE_ADDITIVESINE_H
