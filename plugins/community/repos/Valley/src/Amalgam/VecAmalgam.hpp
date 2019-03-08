//
//  VecAmalgam.hpp
//  BitCombiner
//
//  Created by Dale Johnson on 26/11/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#ifndef VecAmalgam_hpp
#define VecAmalgam_hpp
#include <ctime>
#include <cmath>
#include "../Common/Utilities.hpp"
#include "../Common/SIMD/SIMDUtilities.hpp"
#include "../Common/SIMD/VecOnePoleFilters.hpp"
#include "VecDiodeRingMod.hpp"

class VecAmalgam {
public:
    enum {
        RING_MOD_1_MODE,
        RING_MOD_2_MODE,
        RING_MOD_3_MODE,
        DIODE_RING_MOD_MODE,
        MIN_MAX_MODE,
        SIGN_SWITCH_1_MODE,
        SIGN_SWITCH_2_MODE,
        X_FADE_MODE,
        FLIP_FLOP_MODE,
        ALPHA_PWM_MODE,
        BIT_AND_MODE,
        BIT_XOR_MODE,
        BIT_INTERLEAVE_MODE,
        BIT_HACK_MODE,

        BIT_AND_FLOAT_MODE,
        BIT_INTERLEAVE_FLOAT_MODE,
        BIT_HACK_FLOAT_MODE,
        NUM_MODES
    } Modes;

    VecAmalgam();

    inline __m128 process(const __m128& x, const __m128& y, float paramA, float paramB) {
        // Calculate sample clock for bit wise ops
        _updateRate = 1.f - paramB;
        calcStepSize();
        __sample = __zeros;
        _step += _stepSize;
        if(_step >= 1.f) {
            _step -= 1.f;
            __sample = __high;
        }

        __x = x;
        __y = y;
        return (this->*p[_mode])(__x, __y, paramA, paramB);
    }

    void setMode(int mode);
    void setSampleRate(float sampleRate);

private:
    int _mode;
    __m128 __x, __y, __z;
    __m128 __zeros, __ones, __negOnes, __halfs, __high;

    // Ring Mod
    __m128 __xFolded, __yFolded, __xLogic, __yLogic, __zLogic;

    // Flip Flop
    __m128 __ffTarget;
    __m128 __xPrev, __yPrev, __xREdge, __yREdge;
    __m128 __chanceX, __chanceY;
    uint32_t _z[4];
    uint32_t _w[4];
    float _k[4];

    VecDiodeRingMod _d;

    // 32 bit mode vars
    int _k32[4];
    __m128 __chance32;
    __m128i __a32, __b32, __c32;

    // 16 bit mode vars
    __m64 __zero16, __one16;
    __m64 __a, __b, __c;
    __m64 __aPrev, __bPrev, __aREdge, __bREdge;
    __m64 __count;

    // Sample reduction
    float _step, _stepSize, _engineSampleRate, _internalSampleRate;
    float _quarterNyquist, _updateRate;
    __m128 __sample;
    __m128 __xDS, __yDS, __zDS;

    void calcStepSize();

    __m128 (VecAmalgam::*p[NUM_MODES])(const __m128& x, const __m128& y, float paramA, float paramB);

    __m128 ringMod1(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 ringMod2(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 ringMod3(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 diodeRingMod(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 minMax(const __m128& x, const __m128& y,float paramA, float paramB);
    __m128 signSwitch1(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 signSwitch2(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 xFade(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 flipFlop(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 alphaPWM(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitAND(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitXOR(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitInterleave(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitHack(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitANDFloat(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitInterleaveFloat(const __m128& x, const __m128& y, float paramA, float paramB);
    __m128 bitHackFloat(const __m128& x, const __m128& y, float paramA, float paramB);

    inline void downSample(const __m128& x, const __m128& y) {
        __xDS = _mm_switch_ps(__xDS, x, __sample);
        __yDS = _mm_switch_ps(__yDS, y, __sample);
    }
};
#endif /* VecAmalgam_hpp */
