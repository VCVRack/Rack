#ifndef WAVETABLE_VGAME_H
#define WAVETABLE_VGAME_H
#define WAVETABLE_VGAME_NUM 8

#include "vgame_1.h"
#include "vgame_2.h"
#include "vgame_3.h"
#include "vgame_4.h"
#include "vgame_5.h"
#include "vgame_6.h"
#include "vgame_7.h"
#include "vgame_8.h"

static float* wavetable_vgame[WAVETABLE_VGAME_NUM] = {
    vgame_1_waveTable,
    vgame_2_waveTable,
    vgame_3_waveTable,
    vgame_4_waveTable,
    vgame_5_waveTable,
    vgame_6_waveTable,
    vgame_7_waveTable,
    vgame_8_waveTable
};

static long wavetable_vgame_lengths[WAVETABLE_VGAME_NUM] = {
    vgame_1_tableLength,
    vgame_2_tableLength,
    vgame_3_tableLength,
    vgame_4_tableLength,
    vgame_5_tableLength,
    vgame_6_tableLength,
    vgame_7_tableLength,
    vgame_8_tableLength
};

#endif // WAVETABLE_VGAME_H
