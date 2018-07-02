#ifndef WAVETABLE_PWM_H
#define WAVETABLE_PWM_H
#define WAVETABLE_PWM_NUM 64

#include "pwm_1.h"
#include "pwm_2.h"
#include "pwm_3.h"
#include "pwm_4.h"
#include "pwm_5.h"
#include "pwm_6.h"
#include "pwm_7.h"
#include "pwm_8.h"
#include "pwm_9.h"
#include "pwm_10.h"
#include "pwm_11.h"
#include "pwm_12.h"
#include "pwm_13.h"
#include "pwm_14.h"
#include "pwm_15.h"
#include "pwm_16.h"
#include "pwm_17.h"
#include "pwm_18.h"
#include "pwm_19.h"
#include "pwm_20.h"
#include "pwm_21.h"
#include "pwm_22.h"
#include "pwm_23.h"
#include "pwm_24.h"
#include "pwm_25.h"
#include "pwm_26.h"
#include "pwm_27.h"
#include "pwm_28.h"
#include "pwm_29.h"
#include "pwm_30.h"
#include "pwm_31.h"
#include "pwm_32.h"
#include "pwm_33.h"
#include "pwm_34.h"
#include "pwm_35.h"
#include "pwm_36.h"
#include "pwm_37.h"
#include "pwm_38.h"
#include "pwm_39.h"
#include "pwm_40.h"
#include "pwm_41.h"
#include "pwm_42.h"
#include "pwm_43.h"
#include "pwm_44.h"
#include "pwm_45.h"
#include "pwm_46.h"
#include "pwm_47.h"
#include "pwm_48.h"
#include "pwm_49.h"
#include "pwm_50.h"
#include "pwm_51.h"
#include "pwm_52.h"
#include "pwm_53.h"
#include "pwm_54.h"
#include "pwm_55.h"
#include "pwm_56.h"
#include "pwm_57.h"
#include "pwm_58.h"
#include "pwm_59.h"
#include "pwm_60.h"
#include "pwm_61.h"
#include "pwm_62.h"
#include "pwm_63.h"
#include "pwm_64.h"

static float* wavetable_pwm[WAVETABLE_PWM_NUM] = {
    pwm_1_waveTable,
    pwm_2_waveTable,
    pwm_3_waveTable,
    pwm_4_waveTable,
    pwm_5_waveTable,
    pwm_6_waveTable,
    pwm_7_waveTable,
    pwm_8_waveTable,
    pwm_9_waveTable,
    pwm_10_waveTable,
    pwm_11_waveTable,
    pwm_12_waveTable,
    pwm_13_waveTable,
    pwm_14_waveTable,
    pwm_15_waveTable,
    pwm_16_waveTable,
    pwm_17_waveTable,
    pwm_18_waveTable,
    pwm_19_waveTable,
    pwm_20_waveTable,
    pwm_21_waveTable,
    pwm_22_waveTable,
    pwm_23_waveTable,
    pwm_24_waveTable,
    pwm_25_waveTable,
    pwm_26_waveTable,
    pwm_27_waveTable,
    pwm_28_waveTable,
    pwm_29_waveTable,
    pwm_30_waveTable,
    pwm_31_waveTable,
    pwm_32_waveTable,
    pwm_33_waveTable,
    pwm_34_waveTable,
    pwm_35_waveTable,
    pwm_36_waveTable,
    pwm_37_waveTable,
    pwm_38_waveTable,
    pwm_39_waveTable,
    pwm_40_waveTable,
    pwm_41_waveTable,
    pwm_42_waveTable,
    pwm_43_waveTable,
    pwm_44_waveTable,
    pwm_45_waveTable,
    pwm_46_waveTable,
    pwm_47_waveTable,
    pwm_48_waveTable,
    pwm_49_waveTable,
    pwm_50_waveTable,
    pwm_51_waveTable,
    pwm_52_waveTable,
    pwm_53_waveTable,
    pwm_54_waveTable,
    pwm_55_waveTable,
    pwm_56_waveTable,
    pwm_57_waveTable,
    pwm_58_waveTable,
    pwm_59_waveTable,
    pwm_60_waveTable,
    pwm_61_waveTable,
    pwm_62_waveTable,
    pwm_63_waveTable,
    pwm_64_waveTable
};

static long wavetable_pwm_lengths[WAVETABLE_PWM_NUM] = {
    pwm_1_tableLength,
    pwm_2_tableLength,
    pwm_3_tableLength,
    pwm_4_tableLength,
    pwm_5_tableLength,
    pwm_6_tableLength,
    pwm_7_tableLength,
    pwm_8_tableLength,
    pwm_9_tableLength,
    pwm_10_tableLength,
    pwm_11_tableLength,
    pwm_12_tableLength,
    pwm_13_tableLength,
    pwm_14_tableLength,
    pwm_15_tableLength,
    pwm_16_tableLength,
    pwm_17_tableLength,
    pwm_18_tableLength,
    pwm_19_tableLength,
    pwm_20_tableLength,
    pwm_21_tableLength,
    pwm_22_tableLength,
    pwm_23_tableLength,
    pwm_24_tableLength,
    pwm_25_tableLength,
    pwm_26_tableLength,
    pwm_27_tableLength,
    pwm_28_tableLength,
    pwm_29_tableLength,
    pwm_30_tableLength,
    pwm_31_tableLength,
    pwm_32_tableLength,
    pwm_33_tableLength,
    pwm_34_tableLength,
    pwm_35_tableLength,
    pwm_36_tableLength,
    pwm_37_tableLength,
    pwm_38_tableLength,
    pwm_39_tableLength,
    pwm_40_tableLength,
    pwm_41_tableLength,
    pwm_42_tableLength,
    pwm_43_tableLength,
    pwm_44_tableLength,
    pwm_45_tableLength,
    pwm_46_tableLength,
    pwm_47_tableLength,
    pwm_48_tableLength,
    pwm_49_tableLength,
    pwm_50_tableLength,
    pwm_51_tableLength,
    pwm_52_tableLength,
    pwm_53_tableLength,
    pwm_54_tableLength,
    pwm_55_tableLength,
    pwm_56_tableLength,
    pwm_57_tableLength,
    pwm_58_tableLength,
    pwm_59_tableLength,
    pwm_60_tableLength,
    pwm_61_tableLength,
    pwm_62_tableLength,
    pwm_63_tableLength,
    pwm_64_tableLength
};

#endif // WAVETABLE_PWM_H
