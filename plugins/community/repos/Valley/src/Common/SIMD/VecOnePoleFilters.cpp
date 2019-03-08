#include "VecOnePoleFilters.hpp"

VecOnePoleLPFilter::VecOnePoleLPFilter() {
    setSampleRate(44100.f);
    setCutoffFreq(_sampleRate / 2.f);
    clear();
}

VecOnePoleLPFilter::VecOnePoleLPFilter(float cutoffFreq) {
    setSampleRate(44100.f);
    setCutoffFreq(cutoffFreq);
    clear();
}

__m128 VecOnePoleLPFilter::process(const __m128& input) {
    _z =  _mm_add_ps(_mm_mul_ps(_a, input), _mm_mul_ps(_z, _b));
    return _z;
}

void VecOnePoleLPFilter::clear() {
    _z = _mm_set1_ps(0.f);
}

void VecOnePoleLPFilter::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
    _1_sampleRate = 1.f / sampleRate;
    _maxCutoffFreq = sampleRate / 2.f - 2.f;
    setCutoffFreq(_cutoffFreq);
}

void VecOnePoleLPFilter::setCutoffFreq(float cutoffFreq) {
    _cutoffFreq = cutoffFreq > _maxCutoffFreq ? _maxCutoffFreq : cutoffFreq;
    _b = _mm_set1_ps(expf(-_2M_PI * _cutoffFreq * _1_sampleRate));
    _a = _mm_sub_ps(_mm_set1_ps(1.f), _b);
}

float VecOnePoleLPFilter::getMaxCutoffFreq() const {
    return _maxCutoffFreq;
}

///////////////////////////////////////////////////////////////////////////////

VecOnePoleHPFilter::VecOnePoleHPFilter() {
    setSampleRate(44100.f);
    setCutoffFreq(_sampleRate / 2.f);
    clear();
    _bypassed = false;
}

VecOnePoleHPFilter::VecOnePoleHPFilter(float cutoffFreq) {
    setSampleRate(44100.f);
    setCutoffFreq(cutoffFreq);
    clear();
    _bypassed = false;
}

__m128 VecOnePoleHPFilter::process(const __m128& input) {
    _z =  _mm_add_ps(_mm_mul_ps(_a, input), _mm_mul_ps(_z, _b));
    return _bypassed ? input : _mm_sub_ps(input, _z);
}

void VecOnePoleHPFilter::clear() {
    _z = _mm_set1_ps(0.f);
}

void VecOnePoleHPFilter::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
    _1_sampleRate = 1.f / sampleRate;
    _maxCutoffFreq = sampleRate / 2.f - 2.f;
    setCutoffFreq(_cutoffFreq);
}

void VecOnePoleHPFilter::setCutoffFreq(float cutoffFreq) {
    _cutoffFreq = cutoffFreq > _maxCutoffFreq ? _maxCutoffFreq : cutoffFreq;
    _b = _mm_set1_ps(expf(-_2M_PI * _cutoffFreq * _1_sampleRate));
    _a = _mm_sub_ps(_mm_set1_ps(1.f), _b);
}

float VecOnePoleHPFilter::getMaxCutoffFreq() const {
    return _maxCutoffFreq;
}

void VecOnePoleHPFilter::setBypass(bool bypass) {
    _bypassed = bypass;
}
