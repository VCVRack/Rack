#ifndef WAVETABLE_ALTOSAX_H
#define WAVETABLE_ALTOSAX_H
#define WAVETABLE_ALTOSAX_NUM 8

#include "altosax_1.h"
#include "altosax_2.h"
#include "altosax_3.h"
#include "altosax_4.h"
#include "altosax_5.h"
#include "altosax_6.h"
#include "altosax_7.h"
#include "altosax_8.h"

static float* wavetable_altosax[WAVETABLE_ALTOSAX_NUM] = {
    altosax_1_waveTable,
    altosax_2_waveTable,
    altosax_3_waveTable,
    altosax_4_waveTable,
    altosax_5_waveTable,
    altosax_6_waveTable,
    altosax_7_waveTable,
    altosax_8_waveTable
};

static long wavetable_altosax_lengths[WAVETABLE_ALTOSAX_NUM] = {
    altosax_1_tableLength,
    altosax_2_tableLength,
    altosax_3_tableLength,
    altosax_4_tableLength,
    altosax_5_tableLength,
    altosax_6_tableLength,
    altosax_7_tableLength,
    altosax_8_tableLength
};

#endif // WAVETABLE_ALTOSAX_H
