//
//  VecOTAFilter.cpp
//
//  Created by Dale Johnson on 13/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#include "VecOTAFilter.hpp"

VecTPTOnePoleStage::VecTPTOnePoleStage() {
    calcTanhTable();
    _zeros = _mm_set1_ps(0.f);
    _ones = _mm_set1_ps(1.f);
    _G = _zeros;
    _z = _zeros;
    _v = _zeros;
    _out = _zeros;
    setSampleRate(44100.f);
}

void VecTPTOnePoleStage::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
}

float VecTPTOnePoleStage::getSampleRate() const {
    return _sampleRate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VecOTAFilter::VecOTAFilter() {
    __ones = _mm_set1_ps(1.f);
    __zeros = _mm_set1_ps(0.f);

    __G = __zeros;
    __sigma = __zeros;
    __gamma = __zeros;
    __k = __zeros;
    __u = __zeros;
    __h = __ones;
    __lp1 = __zeros;
    __lp2 = __zeros;
    __lp3 = __zeros;
    calcTanhTable();
    _1_tanhf = 1.f / lookUpTanhf(1.f);
    __pitch = __zeros;

    _pos = (int32_t*)aligned_alloc_16(4 * sizeof(int32_t));
    _lowG = (float*)aligned_alloc_16(4 * sizeof(float));
    _highG = (float*)aligned_alloc_16(4 * sizeof(float));

    setSampleRate(44100.f);
    _poles = -1;
    setNumPoles(2);
}

VecOTAFilter::~VecOTAFilter() {
    aligned_free_16(_pos);
    aligned_free_16(_lowG);
    aligned_free_16(_highG);
}

void VecOTAFilter::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
    calcInternalGTable();
    _stage1.setSampleRate(sampleRate);
    _stage2.setSampleRate(sampleRate);
    _stage3.setSampleRate(sampleRate);
    _stage4.setSampleRate(sampleRate);
    setCutoff(__pitch);
}

void VecOTAFilter::setCutoff(const __m128& pitch) {
    __pitch = _mm_clamp_ps(pitch, __zeros, _mm_set1_ps(10.f));
    __cutoff = _mm_mul_ps(__pitch, _mm_set1_ps(100000.f));
    __cutoffI = _mm_cvttps_epi32(__cutoff);
    _mm_store_si128((__m128i*)_pos, __cutoffI);
    __frac = _mm_sub_ps(__cutoff, _mm_cvtepi32_ps(__cutoffI));

    for(auto i = 0; i < 4; ++i) {
        _lowG[i] = _kGTable[_pos[i]];
        _highG[i] = _kGTable[_pos[i] + 1];
    }

    __lowG = _mm_load_ps(_lowG);
    __highG = _mm_load_ps(_highG);
    __g = _mm_linterp_ps(__lowG, __highG, __frac);

    /*long pos = (long)_cutoff;
    float frac = _cutoff - (float)pos;
    _g = linterp(kGTable[pos], kGTable[pos + 1], frac);*/

    __h = _mm_add_ps(__ones, __g);
    __1_h = _mm_div_ps(__ones, __h);
    __G = _mm_mul_ps(__g, __1_h);

    _stage1._G = __G;
    _stage2._G = __G;
    _stage3._G = __G;
    _stage4._G = __G;
    __G2 = _mm_mul_ps(__G, __G);
    __G3 = _mm_mul_ps(__G2, __G);
    __gamma = _mm_mul_ps(__G3, __G);
}

void VecOTAFilter::setQ(const __m128& Q) {
  //_k = 4.f * clip(Q, 0.f, 10.f) / 10.f;
  __k = _mm_mul_ps(_mm_set1_ps(0.4f), _mm_clamp_ps(Q, _mm_set1_ps(0.f), _mm_set1_ps(10.f)));
}

void VecOTAFilter::calcInternalGTable() {
    float f = 0.f;
    float wd = 0.f;
    float T = 1.f / _sampleRate;
    float T_2 = T / 2.f;
    float wa = 0.f;
    float g = 0.f;

    for(auto i = 0; i < G_TABLE_SIZE; ++i) {
        f = 440.f * powf(2.f, ((i - 500000.f) / 100000.f));
        wd = 2.f * M_PI * f;
        wa = (2.f / T) * tanf(wd * T_2);
        g = wa * T_2;
        _kGTable[i] = g;
    }
}
