#include "OnePoleFilters.hpp"

OnePoleLPFilter::OnePoleLPFilter() {
    setSampleRate(44100.0);
    setCutoffFreq(_sampleRate / 2.0);
    clear();
}

OnePoleLPFilter::OnePoleLPFilter(double cutoffFreq) {
    setSampleRate(44100.0);
    setCutoffFreq(cutoffFreq);
    clear();
}

double OnePoleLPFilter::process() {
    _z =  _a * input + _z * _b;
    output = _z;
    return output;
}

void OnePoleLPFilter::clear() {
    input = 0.0;
    _z = 0.0;
    output = 0.0;
}

void OnePoleLPFilter::setSampleRate(double sampleRate) {
    _sampleRate = sampleRate;
    _1_sampleRate = 1.0 / sampleRate;
    _maxCutoffFreq = sampleRate / 2.0 - 1.0;
    setCutoffFreq(_cutoffFreq);
}

void OnePoleLPFilter::setCutoffFreq(double cutoffFreq) {
    _cutoffFreq = cutoffFreq;
    _b = expf(-_2M_PI * _cutoffFreq * _1_sampleRate);
    _a = 1.0 - _b;
}

double OnePoleLPFilter::getMaxCutoffFreq() const {
    return _maxCutoffFreq;
}


OnePoleHPFilter::OnePoleHPFilter() {
    _cutoffFreq = 0.0;
    setSampleRate(44100.0);
    setCutoffFreq(_cutoffFreq);
    clear();
}

OnePoleHPFilter::OnePoleHPFilter(double cutoffFreq) {
    _cutoffFreq = cutoffFreq;
    setSampleRate(44100.0);
    setCutoffFreq(_cutoffFreq);
    clear();
}

double OnePoleHPFilter::process() {
    _x0 = input;
    _y0 = _a0 * _x0 + _a1 * _x1 + _b1 * _y1;
    _y1 = _y0;
    _x1 = _x0;
    output = _y0;
    return _y0;
}

void OnePoleHPFilter::clear() {
    input = 0.0;
    output = 0.0;
    _x0 = 0.0;
    _x1 = 0.0;
    _y0 = 0.0;
    _y1 = 0.0;
}

void OnePoleHPFilter::setCutoffFreq(double cutoffFreq) {
    _cutoffFreq = cutoffFreq;
    _b1 = expf(-_2M_PI * _cutoffFreq * _1_sampleRate);
    _a0 = (1.0 + _b1) / 2.0;
    _a1 = -_a0;
}

void OnePoleHPFilter::setSampleRate(double sampleRate) {
    _sampleRate = sampleRate;
    _1_sampleRate = 1.0 / _sampleRate;
    setCutoffFreq(_cutoffFreq);
    clear();
}

int DCBlocker::i = 0;

DCBlocker::DCBlocker() {
    id = i;
    i++;
    setSampleRate(44100.0);
    setCutoffFreq(_sampleRate / 2.0);
    clear();
}

DCBlocker::DCBlocker(double cutoffFreq) {
    setSampleRate(44100.0);
    setCutoffFreq(cutoffFreq);
    clear();
}

double DCBlocker::process(double input) {
    output = input - _z + _b * output;
    _z = input;
    return output;
}

void DCBlocker::clear() {
    _z = 0.0;
    output = 0.0;
}

void DCBlocker::setSampleRate(double sampleRate) {
    _sampleRate = sampleRate;
    _maxCutoffFreq = sampleRate / 2.0;
    setCutoffFreq(_cutoffFreq);
}

void DCBlocker::setCutoffFreq(double cutoffFreq) {
    _cutoffFreq = cutoffFreq;
    _b = 0.999f;
}

double DCBlocker::getMaxCutoffFreq() const {
    return _maxCutoffFreq;
}
