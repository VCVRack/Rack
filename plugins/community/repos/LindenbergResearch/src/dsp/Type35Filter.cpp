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

#include "Type35Filter.hpp"
#include "DSPEffect.hpp"
#include "DSPMath.hpp"


/**
 * @brief Construct a new Korg35 filter stage
 * @param sr SampleRate
 * @param type Lowpass / Highpass
 */
dsp::Type35FilterStage::Type35FilterStage(float sr, FilterType type) : DSPEffect(sr) {
    this->type = type;
}


/**
 * @brief Init stage
 */
void dsp::Type35FilterStage::init() {
    type = LP_STAGE;
    alpha = 1.f;
    beta = 1.f;
    zn1 = 0;
    fc = 0;
}


/**
 * @brief Recompute filter parameter
 */
void dsp::Type35FilterStage::invalidate() {
    // only process in dedicated mode
    if (!dedicated) return;

    float wd = TWOPI * fc;
    float T = 1.f / sr;
    float wa = (2.f / T) * tanf(wd * T / 2.f);
    float g = wa * T / 2.f;

    alpha = g / (1.f + g);
}


/**
 * @brief Update filter and compute next sample
 */
void dsp::Type35FilterStage::process() {
    // v(n)
    float vn = (in - zn1) * alpha;

    float lpf = vn + zn1;

    zn1 = vn + lpf;

    float hpf = in - lpf;

    // switch lpf type
    if (type == LP_STAGE) {
        out = lpf;
    } else {
        out = hpf;
    }

}


/**
 * @brief Init main filter
 */
void dsp::Type35Filter::init() {
    fc = sr / 2.f;
    peak = 0.f;



    /* lowpass stages */
    lpf1->init();
    lpf2->init();

    /* highpass stages */
    hpf1->init();
    hpf2->init();
}


/**
 * @brief Recompute filter parameter
 */
void dsp::Type35Filter::invalidate() {
    float frqHz;

    fc = clamp(fc, 0.f, 1.1f);
    peak = clamp(peak, 0.0001, 1.1f);

    if (type == LPF)
        frqHz = (MAX_FREQUENCY / 1000.f) * powf(950.f, fc) - 20.f;
    else
        frqHz = (MAX_FREQUENCY / 1000.f) * powf(1000.f, fc);

    peak = cubicShape(peak) * 2.f + noise.nextFloat(10e-7);

    float wd = TWOPI * frqHz;
    float T = 1.f / sr;
    float wa = (2.f / T) * tanf(wd * T / 2.f);
    float g = wa * T / 2.f;

    float G = g / (1.f + g);


    if (type == HPF) {
        /* HIGHPASS */
        lpf1->alpha = G;
        hpf1->alpha = G;
        hpf2->alpha = G;

        hpf2->beta = -1.f * G / (1.f + g);
        lpf1->beta = 1.f / (1.f + g);
    } else {
        /* LOWPASS */
        lpf1->alpha = G;
        lpf2->alpha = G;
        hpf1->alpha = G;

        lpf2->beta = (peak - peak * G) / (1.f + g);
        hpf1->beta = -1.f / (1.f + g);
    }

    Ga = 1.f / (1.f - peak * G + peak * G * G);
}


/**
 * @brief Compute next sample for output depending on filter type
 */
void dsp::Type35Filter::process() {
    type == LPF ? processLPF() : processHPF();
}


/**
 * @brief Do the lowpass filtering and oversampling
 */
void dsp::Type35Filter::processLPF() {
    lpf1->in = in + noise.nextFloat(NOISE_GAIN);;
    lpf1->process();
    float y1 = lpf1->out;

    float s35h = hpf1->getFeedback() + lpf2->getFeedback();

    float u = Ga * (y1 + s35h);
    //float y = peak * fastatan(sat * u * 0.1) * 10.f;

    u = fastatan(sat * u * 0.1) * 10.f;

    lpf2->in = u;
    lpf2->process();

    float y = peak * lpf2->out;


    hpf1->in = y;
    hpf1->process();


    if (peak > 0) {
        y *= 1.f / peak; // normalize
    }

    out = y;
}


/**
 * @brief Do the highpass filtering and oversampling
 */
void dsp::Type35Filter::processHPF() {
    hpf1->in = in + noise.nextFloat(NOISE_GAIN);
    hpf1->process();
    float y1 = hpf1->out;

    float s35h = hpf2->getFeedback() + lpf1->getFeedback();

    float u = Ga * (y1 + s35h);
    float y = peak * fastatan(sat * u * 0.1) * 10.f;

    hpf2->in = y;
    hpf2->process();

    lpf1->in = hpf2->out;
    lpf1->process();

    if (peak > 0) {
        y *= 1.f / peak; // normalize
    }

    out = y;
}


/**
 * @brief Update samplerate
 * @param sr SR
 */
void dsp::Type35Filter::setSamplerate(float sr) {
    DSPEffect::setSamplerate(sr * OVERSAMPLE);

    // derive samplerate change
    lpf1->setSamplerate(sr * OVERSAMPLE);
    lpf2->setSamplerate(sr * OVERSAMPLE);
    hpf1->setSamplerate(sr * OVERSAMPLE);
    hpf2->setSamplerate(sr * OVERSAMPLE);

    invalidate();
}


/**
 * @brief Top function which handles the oversampling
 */
void dsp::Type35Filter::process2() {
    rs->doUpsample(IN, in);

    for (int i = 0; i < rs->getFactor(); i++) {
        in = (float) rs->getUpsampled(IN)[i];

        process();

        rs->data[IN][i] = out;
    }

    out = (float) rs->getDownsampled(IN);;
}
