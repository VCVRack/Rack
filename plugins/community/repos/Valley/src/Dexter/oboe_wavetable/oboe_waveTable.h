#ifndef WAVETABLE_OBOE_H
#define WAVETABLE_OBOE_H
#define WAVETABLE_OBOE_NUM 13

#include "oboe_1.h"
#include "oboe_2.h"
#include "oboe_3.h"
#include "oboe_4.h"
#include "oboe_5.h"
#include "oboe_6.h"
#include "oboe_7.h"
#include "oboe_8.h"
#include "oboe_9.h"
#include "oboe_10.h"
#include "oboe_11.h"
#include "oboe_12.h"
#include "oboe_13.h"

static float* wavetable_oboe[13] = {
    oboe_1_waveTable,
    oboe_2_waveTable,
    oboe_3_waveTable,
    oboe_4_waveTable,
    oboe_5_waveTable,
    oboe_6_waveTable,
    oboe_7_waveTable,
    oboe_8_waveTable,
    oboe_9_waveTable,
    oboe_10_waveTable,
    oboe_11_waveTable,
    oboe_12_waveTable,
    oboe_13_waveTable
};

static long wavetable_oboe_lengths[13] = {
    oboe_1_tableLength,
    oboe_2_tableLength,
    oboe_3_tableLength,
    oboe_4_tableLength,
    oboe_5_tableLength,
    oboe_6_tableLength,
    oboe_7_tableLength,
    oboe_8_tableLength,
    oboe_9_tableLength,
    oboe_10_tableLength,
    oboe_11_tableLength,
    oboe_12_tableLength,
    oboe_13_tableLength
};
#endif // WAVETABLE_OBOE_H
