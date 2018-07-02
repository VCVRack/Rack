#ifndef WAVETABLE_VOICE2_H
#define WAVETABLE_VOICE2_H
#define WAVETABLE_VOICE2_NUM 8

#include "voice2_1.h"
#include "voice2_2.h"
#include "voice2_3.h"
#include "voice2_4.h"
#include "voice2_5.h"
#include "voice2_6.h"
#include "voice2_7.h"
#include "voice2_8.h"

static float* wavetable_voice2[WAVETABLE_VOICE2_NUM] = {
    voice2_1_waveTable,
    voice2_2_waveTable,
    voice2_3_waveTable,
    voice2_4_waveTable,
    voice2_5_waveTable,
    voice2_6_waveTable,
    voice2_7_waveTable,
    voice2_8_waveTable
};

static long wavetable_voice2_lengths[WAVETABLE_VOICE2_NUM] = {
    voice2_1_tableLength,
    voice2_2_tableLength,
    voice2_3_tableLength,
    voice2_4_tableLength,
    voice2_5_tableLength,
    voice2_6_tableLength,
    voice2_7_tableLength,
    voice2_8_tableLength
};

#endif // WAVETABLE_VOICE2_H
