/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */

#include "Korg35Filter.hpp"
#include "DSPEffect.hpp"


dsp::Korg35FilterStage::Korg35FilterStage(float sr, FilterType type) : DSPEffect(sr) {
    this->type = type;
}


void dsp::Korg35FilterStage::init() {
    type = LPF1;
    alpha = 1.f;
    beta = 1.f;
    zn1 = 0;
    fc = 0;
}


void dsp::Korg35FilterStage::invalidate() {
    // only process in dedicated mode
    if (!dedicated) return;

    float wd = 2 * PI * fc;
    float T = 1 / sr;
    float wa = (2 / T) * tan(wd * T / 2);
    float g = wa * T / 2;

    alpha = g / (1.f + g);
}


void dsp::Korg35FilterStage::process() {
    // v(n)
    float vn = (in - zn1) * alpha;

    float lpf = vn + zn1;

    zn1 = vn + lpf;

    // switch filter type
    if (type == LPF1) {
        out = lpf;
    } else {
        float hpf = in - lpf;
        out = hpf;
    }

}


void dsp::Korg35Filter::init() {
    fc = sr / 2;
    peak = 0.f;

    lpf->init();
    hpf1->init();
    hpf2->init();
}


void dsp::Korg35Filter::invalidate() {
    float frqHz = MAX_FREQUENCY / 1000.f * powf(1000.f, fc);

    float wd = 2 * PI * frqHz;
    float T = 1 / sr;
    float wa = (2 / T) * tan(wd * T / 2);
    float g = wa * T / 2;

    float G = g / (1.f + g);

    // set alphas
    lpf->alpha = G;
    hpf1->alpha = G;
    hpf2->alpha = G;

    hpf2->beta = -1.f * G / (1.f + g);
    lpf->beta = 1.f / (1.f + g);

    Ga = 1.f / (1.f - peak * G + peak * G * G);
}


void dsp::Korg35Filter::process() {
    hpf1->in = in;
    hpf1->process();
    float y1 = hpf1->out;

    float s35h = hpf2->getFeedback() + lpf->getFeedback();

    float u = Ga * (y1 + s35h);
    float y = peak * u;

    y = tanh(sat * y);

    hpf2->in = y;
    hpf2->process();

    lpf->in = hpf2->out;
    lpf->process();

    if (peak > 0) {
        y *= 1 / peak; // normalize
    }

    out = y;
}


void dsp::Korg35Filter::setSamplerate(float sr) {
    DSPEffect::setSamplerate(sr);

    // derive samplerate change
    lpf->setSamplerate(sr);
    hpf1->setSamplerate(sr);
    hpf2->setSamplerate(sr);

    invalidate();
}
