#ifndef WAVETABLE_VOICE4_H
#define WAVETABLE_VOICE4_H
#define WAVETABLE_VOICE4_NUM 8

#include "voice4_1.h"
#include "voice4_2.h"
#include "voice4_3.h"
#include "voice4_4.h"
#include "voice4_5.h"
#include "voice4_6.h"
#include "voice4_7.h"
#include "voice4_8.h"

static float* wavetable_voice4[WAVETABLE_VOICE4_NUM] = {
    voice4_1_waveTable,
    voice4_2_waveTable,
    voice4_3_waveTable,
    voice4_4_waveTable,
    voice4_5_waveTable,
    voice4_6_waveTable,
    voice4_7_waveTable,
    voice4_8_waveTable
};

static long wavetable_voice4_lengths[WAVETABLE_VOICE4_NUM] = {
    voice4_1_tableLength,
    voice4_2_tableLength,
    voice4_3_tableLength,
    voice4_4_tableLength,
    voice4_5_tableLength,
    voice4_6_tableLength,
    voice4_7_tableLength,
    voice4_8_tableLength
};

#endif // WAVETABLE_VOICE4_H
