#ifndef WAVETABLE_CHIP2_H
#define WAVETABLE_CHIP2_H
#define WAVETABLE_CHIP2_NUM 8

#include "chip2_1.h"
#include "chip2_2.h"
#include "chip2_3.h"
#include "chip2_4.h"
#include "chip2_5.h"
#include "chip2_6.h"
#include "chip2_7.h"
#include "chip2_8.h"

static float* wavetable_chip2[WAVETABLE_CHIP2_NUM] = {
    chip2_1_waveTable,
    chip2_2_waveTable,
    chip2_3_waveTable,
    chip2_4_waveTable,
    chip2_5_waveTable,
    chip2_6_waveTable,
    chip2_7_waveTable,
    chip2_8_waveTable
};

static long wavetable_chip2_lengths[WAVETABLE_CHIP2_NUM] = {
    chip2_1_tableLength,
    chip2_2_tableLength,
    chip2_3_tableLength,
    chip2_4_tableLength,
    chip2_5_tableLength,
    chip2_6_tableLength,
    chip2_7_tableLength,
    chip2_8_tableLength
};

#endif // WAVETABLE_CHIP2_H
