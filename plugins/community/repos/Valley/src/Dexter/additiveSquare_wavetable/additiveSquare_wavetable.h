#ifndef WAVETABLE_ADDITIVESQUARE_H
#define WAVETABLE_ADDITIVESQUARE_H
#define WAVETABLE_ADDITIVESQUARE_NUM 16

#include "additiveSquare_1.h"
#include "additiveSquare_2.h"
#include "additiveSquare_3.h"
#include "additiveSquare_4.h"
#include "additiveSquare_5.h"
#include "additiveSquare_6.h"
#include "additiveSquare_7.h"
#include "additiveSquare_8.h"
#include "additiveSquare_9.h"
#include "additiveSquare_10.h"
#include "additiveSquare_11.h"
#include "additiveSquare_12.h"
#include "additiveSquare_13.h"
#include "additiveSquare_14.h"
#include "additiveSquare_15.h"
#include "additiveSquare_16.h"

static float* wavetable_additiveSquare[WAVETABLE_ADDITIVESQUARE_NUM] = {
    additiveSquare_1_waveTable,
    additiveSquare_2_waveTable,
    additiveSquare_3_waveTable,
    additiveSquare_4_waveTable,
    additiveSquare_5_waveTable,
    additiveSquare_6_waveTable,
    additiveSquare_7_waveTable,
    additiveSquare_8_waveTable,
    additiveSquare_9_waveTable,
    additiveSquare_10_waveTable,
    additiveSquare_11_waveTable,
    additiveSquare_12_waveTable,
    additiveSquare_13_waveTable,
    additiveSquare_14_waveTable,
    additiveSquare_15_waveTable,
    additiveSquare_16_waveTable
};

static long wavetable_additiveSquare_lengths[WAVETABLE_ADDITIVESQUARE_NUM] = {
    additiveSquare_1_tableLength,
    additiveSquare_2_tableLength,
    additiveSquare_3_tableLength,
    additiveSquare_4_tableLength,
    additiveSquare_5_tableLength,
    additiveSquare_6_tableLength,
    additiveSquare_7_tableLength,
    additiveSquare_8_tableLength,
    additiveSquare_9_tableLength,
    additiveSquare_10_tableLength,
    additiveSquare_11_tableLength,
    additiveSquare_12_tableLength,
    additiveSquare_13_tableLength,
    additiveSquare_14_tableLength,
    additiveSquare_15_tableLength,
    additiveSquare_16_tableLength
};

#endif // WAVETABLE_ADDITIVESQUARE_H
