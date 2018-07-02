#ifndef WAVETABLE_VIOLIN_H
#define WAVETABLE_VIOLIN_H
#define WAVETABLE_VIOLIN_NUM 14

#include "violin_1.h"
#include "violin_2.h"
#include "violin_3.h"
#include "violin_4.h"
#include "violin_5.h"
#include "violin_6.h"
#include "violin_7.h"
#include "violin_8.h"
#include "violin_9.h"
#include "violin_10.h"
#include "violin_11.h"
#include "violin_12.h"
#include "violin_13.h"
#include "violin_14.h"

static float* wavetable_violin[WAVETABLE_VIOLIN_NUM] = {
    violin_1_waveTable,
    violin_2_waveTable,
    violin_3_waveTable,
    violin_4_waveTable,
    violin_5_waveTable,
    violin_6_waveTable,
    violin_7_waveTable,
    violin_8_waveTable,
    violin_9_waveTable,
    violin_10_waveTable,
    violin_11_waveTable,
    violin_12_waveTable,
    violin_13_waveTable,
    violin_14_waveTable
};

static long wavetable_violin_lengths[WAVETABLE_VIOLIN_NUM] = {
    violin_1_tableLength,
    violin_2_tableLength,
    violin_3_tableLength,
    violin_4_tableLength,
    violin_5_tableLength,
    violin_6_tableLength,
    violin_7_tableLength,
    violin_8_tableLength,
    violin_9_tableLength,
    violin_10_tableLength,
    violin_11_tableLength,
    violin_12_tableLength,
    violin_13_tableLength,
    violin_14_tableLength
};

#endif // WAVETABLE_VIOLIN_H
