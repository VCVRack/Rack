//
//  OTAFilter.cpp
//
//  Created by Dale Johnson on 13/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#include "OTAFilter.hpp"

void calcGTable(float sampleRate) {
    float f = 0.f;
    float wd = 0.f;
    float T = 1.f / sampleRate;
    float T_2 = T / 2.f;
    float wa = 0.f;
    float g = 0.f;

    for(auto i = 0; i < G_TABLE_SIZE; ++i) {
        f = 440.f * powf(2.f, ((i - 500000.f) / 100000.f));
        wd = 2.f * M_PI * f;
        wa = (2.f / T) * tanf(wd * T_2);
        g = wa * T_2;
        kGTable[i] = g;
    }
}

void calcTanhTable() {
    float x = 0.f;
    for(auto i = 0; i < TANH_TABLE_SIZE; ++i) {
        x = (float)i / ((float)TANH_TABLE_SIZE - 1.f);
        x *= 8.f;
        x -= 4.f;
        //kTanhTable[i] = tanhf(x);
        if(x <= -0.69071) {
            kTanhTable[i] = tanhf(2 * (x + 0.266055));
        }
        else if(x > -0.69071 && x < 0.69071) {
            kTanhTable[i] = x;
        }
        else {
            kTanhTable[i] = tanhf(2 * (x - 0.266055));
        }
    }
}

float lookUpTanhf(float x) {
    x = clip(x, -4.f, 4.f);
    x *= 0.125f;
    x += 0.5f;
    x *= TANH_TABLE_SIZE - 1;
    int i = (int)x;
    return linterp(kTanhTable[i], kTanhTable[i + 1], x - (float)i);
}

TPTOnePoleStage::TPTOnePoleStage() {
    calcTanhTable();
    _G = 0.f;
    _s = 0.f;
    _z = 0.f;
    _v = 0.f;
    _out = 0.f;
    setSampleRate(44100.f);
    //_1_tanhf = 1.f / lookUpTanhf(1.f);
    _1_tanhf = 1.f / tanhDriveSignal(1.f, 1.f);
    _nlp = false;
}

void TPTOnePoleStage::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
}

float TPTOnePoleStage::getSampleRate() const {
    return _sampleRate;
}

void TPTOnePoleStage::setNLP(bool nlp) {
    _nlp = nlp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OTAFilter::OTAFilter() {
    _G = 0.f;
    _sigma = 0.f;
    _gamma = 0.f;
    _k = 0.f;
    _u = 0.f;
    _h = 1.f;
    _S1 = 0.f;
    _S2 = 0.f;
    _S3 = 0.f;
    _S4 = 0.f;
    _lp1 = 0.f;
    _lp2 = 0.f;
    _lp3 = 0.f;
    calcTanhTable();
    _1_tanhf = 1.f / lookUpTanhf(1.f);
    _nlp = false;
    _pitch = 0.f;
    _prevPitch = -1.f;
}

void OTAFilter::setSampleRate(float sampleRate) {
    _stage1.setSampleRate(sampleRate);
    _stage2.setSampleRate(sampleRate);
    _stage3.setSampleRate(sampleRate);
    _stage4.setSampleRate(sampleRate);
    setCutoff(_pitch);
}

void OTAFilter::setCutoff(float pitch) {
    _pitch = clip(pitch, 0.f, 10.0f);
    if(_pitch == _prevPitch) {
        return;
    }
    _prevPitch = _pitch;
    _cutoff = _pitch * 100000.f;
    long pos = (long)_cutoff;
    float frac = _cutoff - (float)pos;

    _g = linterp(kGTable[pos], kGTable[pos + 1], frac);
    _h = 1.f + _g;
    _1_h = 1.f / _h;
    _G = _g * _1_h;
    _stage1._G = _G;
    _stage2._G = _G;
    _stage3._G = _G;
    _stage4._G = _G;
    _gamma = _G * _G * _G * _G;
}

void OTAFilter::setQ(float Q) {
  _k = 4.f * clip(Q, 0.f, 10.f) / 10.f;
}

void OTAFilter::setNLP(bool nlp) {
    _nlp = nlp;
    _stage1.setNLP(nlp);
    _stage2.setNLP(nlp);
    _stage3.setNLP(nlp);
    _stage4.setNLP(nlp);
}
