#pragma once

#include "DSPEffect.hpp"
#include "DSPMath.hpp"
#include "HQTrig.hpp"

static const int OVERSAMPLE = 2;
static const int FEEDBACK_LIMITER_GAIN = 25;
namespace dsp {

struct DiodeLadderStage : DSPEffect {

    float fc; // cutoff frq ????
    float alpha, beta, gamma, delta, epsilon;
    float gain;
    float feedback;
    float z1;

    float in, out;

    explicit DiodeLadderStage(float sr);
    void init() override;
    void invalidate() override;
    void process() override;


    void resetZ1() {
        z1 = 0;
    }


    float getZ1() {
        return z1;
    }


    void setFeedback(float fb) {
        feedback = fb;
    }


    float getFeedbackOutput();
};


struct DiodeLadderFilter : DSPEffect {
    static constexpr float NOISE_GAIN = 10e-9f;     // internal noise gain used for self-oscillation
    static constexpr float MAX_RESONANCE = 17.28f;  // max resonance value
    static constexpr float MAX_FREQUENCY = 20000.f; //
    static const int IN = 0;

    float fc, k, saturation, freqHz;

    DiodeLadderStage *lpf1, *lpf2, *lpf3, *lpf4;
    Noise noise;
    Resampler<1> *rs;

    bool low = false;

    float gamma;
    float sg1, sg2, sg3, sg4;
    float in, out, out2;

    explicit DiodeLadderFilter(float sr);
    void init() override;
    void invalidate() override;
    void process() override;

    void process1();
    void process2();


    void setSamplerate(float sr) override;


    void setFrequency(float fc);
    void setResonance(float k);
    void setIn(float in);
    float getOut() const;
    float getOut2() const;
    void setSaturation(float saturation);


    float getFreqHz() {
        return freqHz;
    }


    void reset() {
        lpf1->resetZ1();
        lpf2->resetZ1();
        lpf3->resetZ1();
        lpf4->resetZ1();

        lpf1->setFeedback(0.f);
        lpf2->setFeedback(0.f);
        lpf3->setFeedback(0.f);
        lpf4->setFeedback(0.f);
    }
};

}
