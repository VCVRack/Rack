#ifndef WAVETABLE_VOICE3_H
#define WAVETABLE_VOICE3_H
#define WAVETABLE_VOICE3_NUM 8

#include "voice3_1.h"
#include "voice3_2.h"
#include "voice3_3.h"
#include "voice3_4.h"
#include "voice3_5.h"
#include "voice3_6.h"
#include "voice3_7.h"
#include "voice3_8.h"

static float* wavetable_voice3[WAVETABLE_VOICE3_NUM] = {
    voice3_1_waveTable,
    voice3_2_waveTable,
    voice3_3_waveTable,
    voice3_4_waveTable,
    voice3_5_waveTable,
    voice3_6_waveTable,
    voice3_7_waveTable,
    voice3_8_waveTable
};

static long wavetable_voice3_lengths[WAVETABLE_VOICE3_NUM] = {
    voice3_1_tableLength,
    voice3_2_tableLength,
    voice3_3_tableLength,
    voice3_4_tableLength,
    voice3_5_tableLength,
    voice3_6_tableLength,
    voice3_7_tableLength,
    voice3_8_tableLength
};

#endif // WAVETABLE_VOICE3_H
