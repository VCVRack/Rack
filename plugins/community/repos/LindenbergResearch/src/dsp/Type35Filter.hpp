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

#pragma once


#include "DSPEffect.hpp"
#include "engine.hpp"
#include "DSPMath.hpp"

namespace dsp {

/**
 * @brief Represents one filter stage
 */
struct Type35FilterStage : DSPEffect {
    enum FilterType {
        LP_STAGE,   // lowpass stage
        HP_STAGE    // highpass stage
    };

    bool dedicated = false;

    FilterType type;

    float fc;
    float alpha, beta;
    float zn1;

    float in, out;

    Type35FilterStage(float sr, FilterType type);


    inline float getFeedback() {
        return zn1 * beta;
    }


    void init() override;
    void invalidate() override;
    void process() override;
};


/**
 * @brief Actual Korg35 Filter Class
 */
struct Type35Filter : DSPEffect {
    static constexpr float MAX_FREQUENCY = 20000.f;
    static const int OVERSAMPLE = 4;
    static constexpr float NOISE_GAIN = 10e-9f; // internal noise gain used for self-oscillation
    static const int IN = 0;

    enum FilterType {
        LPF, // lowpass
        HPF  // highpass
    };


    Type35FilterStage *lpf1, *lpf2, *hpf1, *hpf2;
    FilterType type;
    Noise noise;
    Resampler<1> *rs;

    float Ga;

    float in, out;

    // cutofffrq, peak (resonance) and saturation level
    float fc, peak, sat;


    Type35Filter(float sr, FilterType type) : DSPEffect(sr * OVERSAMPLE) {
        Type35Filter::type = type;

        rs = new Resampler<1>(OVERSAMPLE, 8);

        lpf1 = new Type35FilterStage(sr * OVERSAMPLE, Type35FilterStage::LP_STAGE);
        lpf2 = new Type35FilterStage(sr * OVERSAMPLE, Type35FilterStage::LP_STAGE);
        hpf1 = new Type35FilterStage(sr * OVERSAMPLE, Type35FilterStage::HP_STAGE);
        hpf2 = new Type35FilterStage(sr * OVERSAMPLE, Type35FilterStage::HP_STAGE);
    }


    void init() override;
    void invalidate() override;
    void process() override;
    void process2();
    void processLPF();
    void processHPF();
    void setSamplerate(float sr) override;
};

}


