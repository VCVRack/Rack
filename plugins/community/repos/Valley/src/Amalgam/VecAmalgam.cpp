//
//  VecAmalgam.cpp
//  BitCombiner
//
//  Created by Dale Johnson on 26/11/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#include "VecAmalgam.hpp"

VecAmalgam::VecAmalgam() {
    __zeros = _mm_set1_ps(0.f);
    __ones = _mm_set1_ps(1.f);
    __negOnes = _mm_set1_ps(-1.f);
    __halfs = _mm_set1_ps(0.5f);
    __ffTarget = __zeros;
    __high = _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF));

    __x = __zeros;
    __y = __zeros;
    __z = __zeros;
    __xFolded = __zeros;
    __yFolded = __zeros;

    __zero16 = Dmm_set1_pi16(0);
    __one16 = Dmm_set1_pi16(1);
    __a = __zero16;
    __b = __zero16;
    __c = __zero16;
    __aPrev = __zero16;
    __bPrev = __zero16;
    __aREdge = __zero16;
    __bREdge = __zero16;
    __count = Dmm_set1_pi16(0xFFFF);
    _step = 1.f;
    __xDS = __zeros;
    __yDS = __zeros;
    __zDS = __zeros;

    p[RING_MOD_1_MODE] = &VecAmalgam::ringMod1;
    p[RING_MOD_2_MODE] = &VecAmalgam::ringMod2;
    p[RING_MOD_3_MODE] = &VecAmalgam::ringMod3;
    p[DIODE_RING_MOD_MODE] = &VecAmalgam::diodeRingMod;
    p[MIN_MAX_MODE] = &VecAmalgam::minMax;
    p[SIGN_SWITCH_1_MODE] = &VecAmalgam::signSwitch1;
    p[SIGN_SWITCH_2_MODE] = &VecAmalgam::signSwitch2;
    p[X_FADE_MODE] = &VecAmalgam::xFade;
    p[FLIP_FLOP_MODE] = &VecAmalgam::flipFlop;
    p[ALPHA_PWM_MODE] = &VecAmalgam::alphaPWM;
    p[BIT_AND_MODE] = &VecAmalgam::bitAND;
    p[BIT_XOR_MODE] = &VecAmalgam::bitXOR;
    p[BIT_INTERLEAVE_MODE] = &VecAmalgam::bitInterleave;
    p[BIT_HACK_MODE] = &VecAmalgam::bitHack;
    p[BIT_AND_FLOAT_MODE] = &VecAmalgam::bitANDFloat;
    p[BIT_INTERLEAVE_FLOAT_MODE] = &VecAmalgam::bitInterleaveFloat;
    p[BIT_HACK_FLOAT_MODE] = &VecAmalgam::bitHackFloat;

    _mode = RING_MOD_1_MODE;

    std::srand(std::time(NULL));
    for(auto i = 0; i < 4; ++i) {
        _z[i] = std::rand();
        _w[i] = std::rand();
        _k[i] = 0;
    }
    setSampleRate(44100.f);
}

void VecAmalgam::setMode(int mode) {
    _mode = mode;
    if(_mode < 0) {
        _mode = 0;
    }
    else if(_mode >= NUM_MODES) {
        _mode = NUM_MODES - 1;
    }
}

void VecAmalgam::setSampleRate(float sampleRate) {
    _engineSampleRate = sampleRate;
    _quarterNyquist = _engineSampleRate / 32.f;
    calcStepSize();
}

void VecAmalgam::calcStepSize() {
    _internalSampleRate = _quarterNyquist * powf(2.f, (_updateRate * 5.f));
    _stepSize = _internalSampleRate / _engineSampleRate;
}

__m128 VecAmalgam::ringMod1(const __m128& x, const __m128& y, float paramA, float paramB) {
    __z = _mm_or_ps(_mm_cmpgt_ps(y, _mm_set1_ps(paramB * 1.066f)),
                    _mm_cmplt_ps(y, _mm_set1_ps(paramB * -1.066f)));
    __z = _mm_and_ps(y, __z);
    return _mm_mul_ps(_mm_add_ps(_mm_mul_ps(x, _mm_set1_ps(1.f - 0.5f * paramA)),
                                               _mm_set1_ps(paramA * 0.5f)), __z);
}

__m128 VecAmalgam::ringMod2(const __m128& x, const __m128& y, float paramA, float paramB) {
    __xFolded = _mm_add_ps(_mm_mul_ps(x, __halfs), __halfs);
    __yFolded = _mm_add_ps(_mm_mul_ps(y, __halfs), __halfs);
    __xFolded = _mm_mirror_ps(__xFolded, _mm_set1_ps(paramA + 0.00001f));
    __yFolded = _mm_mirror_ps(__yFolded, _mm_set1_ps(paramB + 0.00001f));
    __xFolded = _mm_mul_ps(_mm_add_ps(__xFolded, _mm_set1_ps(-0.5f)),_mm_set1_ps(2.f));
    __yFolded = _mm_mul_ps(_mm_add_ps(__yFolded, _mm_set1_ps(-0.5f)),_mm_set1_ps(2.f));

    return _mm_mul_ps(__xFolded, __yFolded);
}

__m128 VecAmalgam::ringMod3(const __m128& x, const __m128& y, float paramA, float paramB) {
    __xFolded = _mm_add_ps(_mm_mul_ps(x, __halfs), __halfs);
    __yFolded = _mm_add_ps(_mm_mul_ps(y, __halfs), __halfs);
    __xFolded = _mm_mirror_ps(__xFolded, _mm_set1_ps(paramA + 0.00001f));
    __yFolded = _mm_mirror_ps(__yFolded, _mm_set1_ps(paramA + 0.00001f));
    __xFolded = _mm_mul_ps(_mm_add_ps(__xFolded, _mm_set1_ps(-0.5f)),_mm_set1_ps(2.f));
    __yFolded = _mm_mul_ps(_mm_add_ps(__yFolded, _mm_set1_ps(-0.5f)),_mm_set1_ps(-2.f));

    __xLogic = _mm_cmpgt_ps(__xFolded, __zeros);
    __yLogic = _mm_cmpgt_ps(__yFolded, __zeros);
    __zLogic = _mm_xor_ps(__xLogic, __yLogic);
    __z = _mm_and_ps(__zLogic, __ones);
    __z = _mm_add_ps(_mm_mul_ps(__z, _mm_set1_ps(-2.f)), __ones);

    return _mm_linterp_ps(_mm_mul_ps(__xFolded, __yFolded), __z, _mm_set1_ps(paramB));
}

__m128 VecAmalgam::diodeRingMod(const __m128& x, const __m128& y, float paramA, float paramB) {
    return _d.process(x, y, paramA, paramB);
}

__m128 VecAmalgam::minMax(const __m128& x, const __m128& y, float paramA, float paramB) {
    __m128 select = _mm_cmpgt_ps(x, y);
    __m128 max = _mm_switch_ps(y, x, select);
    __m128 min = _mm_switch_ps(x, y, select);
    __m128 delta = _mm_sub_ps(max, min);
    __z = _mm_linterp_ps(min, max, _mm_set1_ps(paramA));
    return _mm_linterp_ps(__z, _mm_mul_ps(__z, delta), _mm_set1_ps(paramB));
}

__m128 VecAmalgam::signSwitch1(const __m128& x, const __m128& y, float paramA, float paramB) {
    __m128 midPoint = _mm_set1_ps(paramA * 2.f - 1.f);
    __m128 thresh = _mm_set1_ps(paramB);
    return _mm_add_ps(_mm_and_ps(x, _mm_cmpge_ps(x, _mm_add_ps(midPoint, thresh))),
                      _mm_and_ps(y, _mm_cmplt_ps(y,_mm_sub_ps(midPoint, thresh))));
}

__m128 VecAmalgam::signSwitch2(const __m128& x, const __m128& y, float paramA, float paramB) {
    __m128 midPoint = _mm_set1_ps(paramA * 2.f - 1.f);
    __m128 thresh = _mm_set1_ps(paramB);
    __z = _mm_switch_ps(__zeros, x, _mm_cmpgt_ps(x, _mm_add_ps(midPoint, thresh)));
    return _mm_switch_ps(__z, y, _mm_cmplt_ps(y, _mm_sub_ps(midPoint, thresh)));
}

__m128 VecAmalgam::xFade(const __m128& x, const __m128& y, float paramA, float paramB) {
    __m128 xScaled = _mm_add_ps(_mm_mul_ps(x, __halfs), __halfs);
    xScaled = _mm_mul_ps(xScaled, _mm_set1_ps(paramB));
    return _mm_linterp_ps(x,y, _mm_clamp_ps(_mm_add_ps(_mm_set1_ps(paramA), xScaled), __zeros, __ones));
}

__m128 VecAmalgam::flipFlop(const __m128& x, const __m128& y, float paramA, float paramB) {
    __m128 thresh = _mm_set1_ps(paramB);
    for(auto i = 0; i < 4; ++i) {
        _k[i] = (float)mwcRand(_z[i], _w[i]) / (float)UINT32_MAX;
    }
    __chanceX = _mm_loadu_ps(_k);
    __chanceX = _mm_and_ps(_mm_cmpgt_ps(__chanceX, _mm_set1_ps(paramA)), __high);

    for(auto i = 0; i < 4; ++i) {
        _k[i] = (float)mwcRand(_z[i], _w[i]) / (float)UINT32_MAX;
    }
    __chanceY = _mm_loadu_ps(_k);
    __chanceY = _mm_and_ps(_mm_cmpgt_ps(__chanceY, _mm_set1_ps(1.f - paramA)), __high);

    __xREdge = _mm_and_ps(_mm_cmpgt_ps(x, thresh), _mm_cmple_ps(__xPrev, thresh));
    __yREdge = _mm_and_ps(_mm_cmpgt_ps(y, thresh), _mm_cmple_ps(__yPrev, thresh));
    __xREdge = _mm_and_ps(__xREdge, __chanceX);
    __yREdge = _mm_and_ps(__yREdge, __chanceY);

    __ffTarget = _mm_switch_ps(__ffTarget, __zeros, __xREdge);
    __ffTarget = _mm_switch_ps(__ffTarget, __high, __yREdge);
    __xPrev = x;
    __yPrev = y;
    return _mm_switch_ps(x, y, __ffTarget);
}

__m128 VecAmalgam::alphaPWM(const __m128& x, const __m128& y, float paramA, float paramB) {
    __z = _mm_mul_ps(_mm_abs_ps(x), __halfs);
    __z = _mm_mul_ps(__z, _mm_add_ps(_mm_mul_ps(_mm_set1_ps(paramA), _mm_set1_ps(16.f)), __ones));
    __m128i xInt = _mm_cvttps_epi32(__z);
    __m128 xIntF = _mm_cvtepi32_ps(xInt);
    __z = _mm_sub_ps(__z, xIntF);
    return _mm_mul_ps(_mm_switch_ps(x, _mm_set1_ps(0.f), _mm_cmpgt_ps(__z, _mm_set1_ps(1.f - paramB))), y);
}

__m128 VecAmalgam::bitAND(const __m128& x, const __m128& y, float paramA, float paramB) {
    paramA = 1.f - paramA * 0.8f;
    paramA *= paramA;
    downSample(_mm_varStep_ps(x, _mm_set1_ps(1.f - paramA)),_mm_varStep_ps(y, _mm_set1_ps(1.f - paramA)));
    __a32 = _mm_cvttps_epi32(_mm_mul_ps(__xDS, _mm_set1_ps(0x7FFFFFFF)));
    __b32 = _mm_cvttps_epi32(_mm_mul_ps(__yDS, _mm_set1_ps(0x7FFFFFFF)));
    __c32 = _mm_and_si128(__a32, __b32);
    return _mm_div_ps(_mm_cvtepi32_ps(__c32), _mm_set1_ps(0x7FFFFFFF));
}

__m128 VecAmalgam::bitXOR(const __m128& x, const __m128& y, float paramA, float paramB) {
    paramA = 1.f - paramA * 0.8f;
    paramA *= paramA;
    downSample(_mm_varStep_ps(x, _mm_set1_ps(1.f - paramA)),_mm_varStep_ps(y, _mm_set1_ps(1.f - paramA)));
    __a32 = _mm_cvttps_epi32(_mm_mul_ps(__xDS, _mm_set1_ps(0x7FFFFFFF)));
    __b32 = _mm_cvttps_epi32(_mm_mul_ps(__yDS, _mm_set1_ps(0x7FFFFFFF)));
    __c32 = _mm_xor_si128(__a32, __b32);
    return _mm_div_ps(_mm_cvtepi32_ps(__c32), _mm_set1_ps(0x7FFFFFFF));
}

__m128 VecAmalgam::bitInterleave(const __m128& x, const __m128& y, float paramA, float paramB) {
    paramA = 1.f - paramA * 0.8f;
    paramA *= paramA;
    downSample(_mm_varStep_ps(x, _mm_set1_ps(1.f - paramA)),_mm_varStep_ps(y, _mm_set1_ps(1.f - paramA)));
    __a32 = _mm_cvttps_epi32(_mm_mul_ps(__xDS, _mm_set1_ps(0x7FFFFFFF)));
    __b32 = _mm_cvttps_epi32(_mm_mul_ps(__yDS, _mm_set1_ps(0x7FFFFFFF)));
    __c32 = _mm_xor_si128(__a32, __b32);
    __c32 = _mm_or_si128(_mm_and_si128(__a32, _mm_set1_epi32(0x55555555)), _mm_and_si128(__b32, _mm_set1_epi32(0xAAAAAAAA)));
    return _mm_div_ps(_mm_cvtepi32_ps(__c32), _mm_set1_ps(0x7FFFFFFF));
}

__m128 VecAmalgam::bitHack(const __m128& x, const __m128& y, float paramA, float paramB) {
    __chance32 = __high;
    int random = 0;
    for(auto i = 0; i < 4; ++i) {
        random = mwcRand(_z[i], _w[i]);
        _k32[i] = (float)random / (float)UINT32_MAX > (0.5f - paramA * paramA * 0.5f) ? random : 0xFFFFFFFF;
    }
    __chance32 = _mm_castsi128_ps(_mm_set_epi32(_k32[3], _k32[2], _k32[1], _k32[0]));

    downSample(x,y);
    __a32 = _mm_cvttps_epi32(_mm_mul_ps(__xDS, _mm_set1_ps(0x7FFFFFFF)));
    __b32 = _mm_cvttps_epi32(_mm_mul_ps(__yDS, _mm_set1_ps(0x7FFFFFFF)));
    __c32 = _mm_and_si128(_mm_or_si128(__c32, _mm_and_si128(__a32, __b32)), _mm_castps_si128(__chance32)); // Make 1 if a == 1 AND b == 1
#ifdef _MSC_VER
    {
       // workaround for MSVC "error C2675: unary '~': '__m128i' does not define this operator or a conversion to a  type acceptable to the predefined operator"
       __m128i a32inv;
       a32inv.m128i_u64[0] = ~__a32.m128i_u64[0];
       a32inv.m128i_u64[1] = ~__a32.m128i_u64[1];
       __m128i b32inv;
       b32inv.m128i_u64[0] = ~__b32.m128i_u64[0];
       b32inv.m128i_u64[1] = ~__b32.m128i_u64[1];
       __m128i tinv = _mm_and_si128(_mm_and_si128(a32inv, b32inv), _mm_castps_si128(__chance32));
       tinv.m128i_u64[0] = ~tinv.m128i_u64[0];
       tinv.m128i_u64[1] = ~tinv.m128i_u64[1];
       __c32 = _mm_and_si128(__c32, tinv); // Make 0 if a == 0 AND b == 0
    }
#else
    __c32 = _mm_and_si128(__c32, ~_mm_and_si128(_mm_and_si128(~__a32, ~__b32), _mm_castps_si128(__chance32))); // Make 0 if a == 0 AND b == 0
#endif
    __z = _mm_div_ps(_mm_cvtepi32_ps(__c32), _mm_set1_ps(0x7FFFFFFF));
    __zDS = _mm_switch_ps(__zDS, __z, __sample);
    return __zDS;
}

__m128 VecAmalgam::bitANDFloat(const __m128& x, const __m128& y, float paramA, float paramB) {
    __chance32 = __high;
    int random = 0;
    for(auto i = 0; i < 4; ++i) {
        random = mwcRand(_z[i], _w[i]);
        _k32[i] = (float)random / (float)UINT32_MAX > (0.5f - paramA * paramA * 0.5f) ? random : 0xFFFFFFFF;
    }
    __chance32 = _mm_castsi128_ps(_mm_set_epi32(_k32[3], _k32[2], _k32[1], _k32[0]));

    downSample(x,y);
    __z = _mm_and_ps(_mm_and_ps(__xDS,__yDS), __chance32);
    __zDS = _mm_switch_ps(__zDS, __z, __sample);
    return __zDS;
}

__m128 VecAmalgam::bitInterleaveFloat(const __m128& x, const __m128& y, float paramA, float paramB) {
    paramA = 1.f - paramA * 0.8f;
    paramA *= paramA;
    downSample(_mm_varStep_ps(x, _mm_set1_ps(1.f - paramA)),_mm_varStep_ps(y, _mm_set1_ps(1.f - paramA)));
    return _mm_or_ps(_mm_and_ps(__xDS, _mm_castsi128_ps(_mm_set1_epi32(0x55555555))),
                     _mm_and_ps(__yDS, _mm_castsi128_ps(_mm_set1_epi32(0xAAAAAAAA))));
}

__m128 VecAmalgam::bitHackFloat(const __m128& x, const __m128& y, float paramA, float paramB) {
    __chance32 = __high;
    int random = 0;
    for(auto i = 0; i < 4; ++i) {
        random = mwcRand(_z[i], _w[i]);
        _k32[i] = (float)random / (float)UINT32_MAX > (0.5f - paramA * paramA * 0.5f) ? random : 0xFFFFFFFF;
    }
    __chance32 = _mm_castsi128_ps(_mm_set_epi32(_k32[3], _k32[2], _k32[1], _k32[0]));

    downSample(x,y);
    __z = _mm_and_ps(_mm_or_ps(__z, _mm_and_ps(__xDS,__yDS)), __chance32);
    __z = _mm_andnot_ps(_mm_and_ps(_mm_and_ps(_mm_xor_ps(__xDS, __high), _mm_xor_ps(__yDS, __high)),__chance32), __z);
    __zDS = _mm_switch_ps(__zDS, __z, __sample);
    return __zDS;
}
