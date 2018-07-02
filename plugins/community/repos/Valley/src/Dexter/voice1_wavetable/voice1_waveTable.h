#ifndef WAVETABLE_VOICE1_H
#define WAVETABLE_VOICE1_H
#define WAVETABLE_VOICE1_NUM 8

#include "voice1_1.h"
#include "voice1_2.h"
#include "voice1_3.h"
#include "voice1_4.h"
#include "voice1_5.h"
#include "voice1_6.h"
#include "voice1_7.h"
#include "voice1_8.h"

static float* wavetable_voice1[WAVETABLE_VOICE1_NUM] = {
    voice1_1_waveTable,
    voice1_2_waveTable,
    voice1_3_waveTable,
    voice1_4_waveTable,
    voice1_5_waveTable,
    voice1_6_waveTable,
    voice1_7_waveTable,
    voice1_8_waveTable
};

static long wavetable_voice1_lengths[WAVETABLE_VOICE1_NUM] = {
    voice1_1_tableLength,
    voice1_2_tableLength,
    voice1_3_tableLength,
    voice1_4_tableLength,
    voice1_5_tableLength,
    voice1_6_tableLength,
    voice1_7_tableLength,
    voice1_8_tableLength
};

#endif // WAVETABLE_VOICE1_H
