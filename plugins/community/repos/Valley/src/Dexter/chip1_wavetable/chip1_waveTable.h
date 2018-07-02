#ifndef WAVETABLE_CHIP1_H
#define WAVETABLE_CHIP1_H
#define WAVETABLE_CHIP1_NUM 8

#include "chip1_1.h"
#include "chip1_2.h"
#include "chip1_3.h"
#include "chip1_4.h"
#include "chip1_5.h"
#include "chip1_6.h"
#include "chip1_7.h"
#include "chip1_8.h"

static float* wavetable_chip1[WAVETABLE_CHIP1_NUM] = {
    chip1_1_waveTable,
    chip1_2_waveTable,
    chip1_3_waveTable,
    chip1_4_waveTable,
    chip1_5_waveTable,
    chip1_6_waveTable,
    chip1_7_waveTable,
    chip1_8_waveTable
};

static long wavetable_chip1_lengths[WAVETABLE_CHIP1_NUM] = {
    chip1_1_tableLength,
    chip1_2_tableLength,
    chip1_3_tableLength,
    chip1_4_tableLength,
    chip1_5_tableLength,
    chip1_6_tableLength,
    chip1_7_tableLength,
    chip1_8_tableLength
};

#endif // WAVETABLE_CHIP1_H
