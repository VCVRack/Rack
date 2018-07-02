#ifndef WAVETABLE_OPAL_H
#define WAVETABLE_OPAL_H
#define WAVETABLE_OPAL_NUM 8

#include "opal_1.h"
#include "opal_2.h"
#include "opal_3.h"
#include "opal_4.h"
#include "opal_5.h"
#include "opal_6.h"
#include "opal_7.h"
#include "opal_8.h"

static float* wavetable_opal[WAVETABLE_OPAL_NUM] = {
    opal_1_waveTable,
    opal_2_waveTable,
    opal_3_waveTable,
    opal_4_waveTable,
    opal_5_waveTable,
    opal_6_waveTable,
    opal_7_waveTable,
    opal_8_waveTable
};

static long wavetable_opal_lengths[WAVETABLE_OPAL_NUM] = {
    opal_1_tableLength,
    opal_2_tableLength,
    opal_3_tableLength,
    opal_4_tableLength,
    opal_5_tableLength,
    opal_6_tableLength,
    opal_7_tableLength,
    opal_8_tableLength
};

#endif // WAVETABLE_OPAL_H
