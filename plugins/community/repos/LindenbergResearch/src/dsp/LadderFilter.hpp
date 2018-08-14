#pragma once


#include "DSPEffect.hpp"
#include "engine.hpp"
#include "DSPMath.hpp"

namespace dsp {

    struct LadderFilter : DSPEffect {

        static const int OVERSAMPLE = 8;                // factor of internal oversampling
        static constexpr float NOISE_GAIN = 10e-10f;    // internal noise gain used for self-oscillation
        static constexpr float INPUT_GAIN = 20.f;       // input level

        enum FXChannel {
            LOWPASS
        };

    private:
        float f, p, q;
        float b0, b1, b2, b3, b4, b5, bx;
        float t1, t2;
        float freqExp, freqHz, frequency, resExp, resonance, drive, slope;
        float in, lpOut;
        float lightValue;

        Resampler<1> *rs;
        Noise noise;

        void updateResExp();

    public:

        LadderFilter(float sr);


        void init() override {
            f = 0;
            p = 0;
            q = 0;
            b0 = 0;
            b1 = 0;
            b2 = 0;
            b3 = 0;
            b4 = 0;
            b5 = 0;
            bx = 0;
            t1 = 0;
            t2 = 0;
            lightValue = 0.0f;
        }


        void invalidate() override;
        void process() override;

        float getFrequency() const;
        void setFrequency(float frequency);
        float getResonance() const;
        void setResonance(float resonance);
        float getDrive() const;
        void setDrive(float drive);
        float getFreqHz() const;

        float getSlope() const;
        void setSlope(float slope);
        void setIn(float in);
        float getLpOut();
        float getLightValue() const;
        void setLightValue(float lightValue);
    };
}