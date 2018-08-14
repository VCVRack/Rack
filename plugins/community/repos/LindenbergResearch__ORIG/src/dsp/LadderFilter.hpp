#pragma once


#include "DSPEffect.hpp"
#include "engine.hpp"
#include "DSPMath.hpp"

namespace rack {

    struct LadderFilter : DSPEffect {

        static const int OVERSAMPLE = 8;                // factor of internal oversampling
        static constexpr float NOISE_GAIN = 10e-10f;    // internal noise gain used for self-oscillation
        static constexpr float INPUT_GAIN = 20.f;            // input level

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

        OverSampler<OVERSAMPLE, 1> os;
        Noise noise;

        void updateResExp();

    public:
        LadderFilter();

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