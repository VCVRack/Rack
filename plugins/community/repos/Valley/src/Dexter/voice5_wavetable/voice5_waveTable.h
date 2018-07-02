#ifndef WAVETABLE_VOICE5_H
#define WAVETABLE_VOICE5_H
#define WAVETABLE_VOICE5_NUM 8

#include "voice5_1.h"
#include "voice5_2.h"
#include "voice5_3.h"
#include "voice5_4.h"
#include "voice5_5.h"
#include "voice5_6.h"
#include "voice5_7.h"
#include "voice5_8.h"

static float* wavetable_voice5[WAVETABLE_VOICE5_NUM] = {
    voice5_1_waveTable,
    voice5_2_waveTable,
    voice5_3_waveTable,
    voice5_4_waveTable,
    voice5_5_waveTable,
    voice5_6_waveTable,
    voice5_7_waveTable,
    voice5_8_waveTable
};

static long wavetable_voice5_lengths[WAVETABLE_VOICE5_NUM] = {
    voice5_1_tableLength,
    voice5_2_tableLength,
    voice5_3_tableLength,
    voice5_4_tableLength,
    voice5_5_tableLength,
    voice5_6_tableLength,
    voice5_7_tableLength,
    voice5_8_tableLength
};

#endif // WAVETABLE_VOICE5_H
