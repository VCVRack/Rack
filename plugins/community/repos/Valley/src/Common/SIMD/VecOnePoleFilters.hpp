#pragma once
#include <cmath>
#include "SIMDUtilities.hpp"
#define _2M_PI 2.0 * M_PI

class VecOnePoleLPFilter {
public:
    VecOnePoleLPFilter();
    VecOnePoleLPFilter(float cutoffFreq);
    __m128 process(const __m128& input);
    void clear();
    void setCutoffFreq(float cutoffFreq);
    void setSampleRate(float sampleRate);
    float getMaxCutoffFreq() const;
private:
    float _sampleRate;
    float _1_sampleRate;
    float _cutoffFreq;
    float _maxCutoffFreq;
    __m128 _a;
    __m128 _b;
    __m128 _z;
};

///////////////////////////////////////////////////////////////////////////////

class VecOnePoleHPFilter {
public:
    VecOnePoleHPFilter();
    VecOnePoleHPFilter(float cutoffFreq);
    __m128 process(const __m128& input);
    void clear();
    void setCutoffFreq(float cutoffFreq);
    void setSampleRate(float sampleRate);
    float getMaxCutoffFreq() const;
    void setBypass(bool bypass);
private:
    float _sampleRate;
    float _1_sampleRate;
    float _cutoffFreq;
    float _maxCutoffFreq;
    bool _bypassed;
    __m128 _a;
    __m128 _b;
    __m128 _z;
};
