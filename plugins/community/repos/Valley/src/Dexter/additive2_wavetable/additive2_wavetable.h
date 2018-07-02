#ifndef WAVETABLE_ADDITIVE2_H
#define WAVETABLE_ADDITIVE2_H
#define WAVETABLE_ADDITIVE2_NUM 8

#include "additive2_1.h"
#include "additive2_2.h"
#include "additive2_3.h"
#include "additive2_4.h"
#include "additive2_5.h"
#include "additive2_6.h"
#include "additive2_7.h"
#include "additive2_8.h"

static float* wavetable_additive2[WAVETABLE_ADDITIVE2_NUM] = {
    additive2_1_waveTable,
    additive2_2_waveTable,
    additive2_3_waveTable,
    additive2_4_waveTable,
    additive2_5_waveTable,
    additive2_6_waveTable,
    additive2_7_waveTable,
    additive2_8_waveTable
};

static long wavetable_additive2_lengths[WAVETABLE_ADDITIVE2_NUM] = {
    additive2_1_tableLength,
    additive2_2_tableLength,
    additive2_3_tableLength,
    additive2_4_tableLength,
    additive2_5_tableLength,
    additive2_6_tableLength,
    additive2_7_tableLength,
    additive2_8_tableLength
};

#endif // WAVETABLE_ADDITIVE2_H
