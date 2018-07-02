#ifndef WAVETABLE_PIANO_H
#define WAVETABLE_PIANO_H
#define WAVETABLE_PIANO_NUM 30

#include "piano_1.h"
#include "piano_2.h"
#include "piano_3.h"
#include "piano_4.h"
#include "piano_5.h"
#include "piano_6.h"
#include "piano_7.h"
#include "piano_8.h"
#include "piano_9.h"
#include "piano_10.h"
#include "piano_11.h"
#include "piano_12.h"
#include "piano_13.h"
#include "piano_14.h"
#include "piano_15.h"
#include "piano_16.h"
#include "piano_17.h"
#include "piano_18.h"
#include "piano_19.h"
#include "piano_20.h"
#include "piano_21.h"
#include "piano_22.h"
#include "piano_23.h"
#include "piano_24.h"
#include "piano_25.h"
#include "piano_26.h"
#include "piano_27.h"
#include "piano_28.h"
#include "piano_29.h"
#include "piano_30.h"

static float* wavetable_piano[30] = {
    piano_1_waveTable,
    piano_2_waveTable,
    piano_3_waveTable,
    piano_4_waveTable,
    piano_5_waveTable,
    piano_6_waveTable,
    piano_7_waveTable,
    piano_8_waveTable,
    piano_9_waveTable,
    piano_10_waveTable,
    piano_11_waveTable,
    piano_12_waveTable,
    piano_13_waveTable,
    piano_14_waveTable,
    piano_15_waveTable,
    piano_16_waveTable,
    piano_17_waveTable,
    piano_18_waveTable,
    piano_19_waveTable,
    piano_20_waveTable,
    piano_21_waveTable,
    piano_22_waveTable,
    piano_23_waveTable,
    piano_24_waveTable,
    piano_25_waveTable,
    piano_26_waveTable,
    piano_27_waveTable,
    piano_28_waveTable,
    piano_29_waveTable,
    piano_30_waveTable
};

static long wavetable_piano_lengths[30] = {
    piano_1_tableLength,
    piano_2_tableLength,
    piano_3_tableLength,
    piano_4_tableLength,
    piano_5_tableLength,
    piano_6_tableLength,
    piano_7_tableLength,
    piano_8_tableLength,
    piano_9_tableLength,
    piano_10_tableLength,
    piano_11_tableLength,
    piano_12_tableLength,
    piano_13_tableLength,
    piano_14_tableLength,
    piano_15_tableLength,
    piano_16_tableLength,
    piano_17_tableLength,
    piano_18_tableLength,
    piano_19_tableLength,
    piano_20_tableLength,
    piano_21_tableLength,
    piano_22_tableLength,
    piano_23_tableLength,
    piano_24_tableLength,
    piano_25_tableLength,
    piano_26_tableLength,
    piano_27_tableLength,
    piano_28_tableLength,
    piano_29_tableLength,
    piano_30_tableLength
};
#endif // WAVETABLE_PIANO_H
