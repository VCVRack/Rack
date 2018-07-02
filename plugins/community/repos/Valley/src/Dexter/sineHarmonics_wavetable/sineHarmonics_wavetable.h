#ifndef WAVETABLE_SINEHARMONICS_H
#define WAVETABLE_SINEHARMONICS_H
#define WAVETABLE_SINEHARMONICS_NUM 16

#include "sineHarmonics_1.h"
#include "sineHarmonics_2.h"
#include "sineHarmonics_3.h"
#include "sineHarmonics_4.h"
#include "sineHarmonics_5.h"
#include "sineHarmonics_6.h"
#include "sineHarmonics_7.h"
#include "sineHarmonics_8.h"
#include "sineHarmonics_9.h"
#include "sineHarmonics_10.h"
#include "sineHarmonics_11.h"
#include "sineHarmonics_12.h"
#include "sineHarmonics_13.h"
#include "sineHarmonics_14.h"
#include "sineHarmonics_15.h"
#include "sineHarmonics_16.h"

static float* wavetable_sineHarmonics[WAVETABLE_SINEHARMONICS_NUM] = {
    sineHarmonics_1_waveTable,
    sineHarmonics_2_waveTable,
    sineHarmonics_3_waveTable,
    sineHarmonics_4_waveTable,
    sineHarmonics_5_waveTable,
    sineHarmonics_6_waveTable,
    sineHarmonics_7_waveTable,
    sineHarmonics_8_waveTable,
    sineHarmonics_9_waveTable,
    sineHarmonics_10_waveTable,
    sineHarmonics_11_waveTable,
    sineHarmonics_12_waveTable,
    sineHarmonics_13_waveTable,
    sineHarmonics_14_waveTable,
    sineHarmonics_15_waveTable,
    sineHarmonics_16_waveTable
};

static long wavetable_sineHarmonics_lengths[WAVETABLE_SINEHARMONICS_NUM] = {
    sineHarmonics_1_tableLength,
    sineHarmonics_2_tableLength,
    sineHarmonics_3_tableLength,
    sineHarmonics_4_tableLength,
    sineHarmonics_5_tableLength,
    sineHarmonics_6_tableLength,
    sineHarmonics_7_tableLength,
    sineHarmonics_8_tableLength,
    sineHarmonics_9_tableLength,
    sineHarmonics_10_tableLength,
    sineHarmonics_11_tableLength,
    sineHarmonics_12_tableLength,
    sineHarmonics_13_tableLength,
    sineHarmonics_14_tableLength,
    sineHarmonics_15_tableLength,
    sineHarmonics_16_tableLength
};

#endif // WAVETABLE_SINEHARMONICS_H
