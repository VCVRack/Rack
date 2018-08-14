#pragma once

#include "DSPSystem.hpp"
#include "DSPMath.hpp"

namespace dsp {


    /**
     * @brief
     */
    struct MS20TPT : DSPSystem2x1 {
        float s = 0;
        DSPDelay1 z;


        void process() override {
            float gx = input[IN1].value * input[IN2].value;

            z.set(gx + z.get() + gx);
            s = z.get();
        }
    };


    /**
     * @brief Zero Delay Feedback
     */
    struct MS20ZDF : DSPSystem2x2 {
        float y = 0;
        float s = 0;
        MS20TPT tpt;


        void process() override {
            y = input[IN1].value * input[IN2].value + s;

            tpt.set(input[IN1].value - y, input[IN2].value);
            s = tpt.s;
        }

    };


    /**
     * @brief MS20 Filter class
     */
    struct MS20zdf : DSPSystem<1, 2, 4> {
        static const int OVERSAMPLE = 8;                // factor of internal oversampling
        static constexpr float DRIVE_GAIN = 20.f;       // max drive gain

        enum Inputs {
            IN
        };

        enum Params {
            FREQUENCY,
            PEAK,
            DRIVE,
            TYPE
        };

        enum Outputs {
            OUT,
        };

    private:
        float g = 0, g2 = 0, b = 0, k = 0;
        float ky = 0, y = 0;
        float freqHz = 0;

        MS20ZDF zdf1, zdf2;
        OverSampler<OVERSAMPLE, 1> os;

    public:
        explicit MS20zdf(float sr);


        float getFrequency() {
            return getParam(FREQUENCY);
        }


        float getFrequencyHz() const {
            return freqHz;
        }


        void setFrequency(float value) {
            setParam(FREQUENCY, clamp(value, 0.f, 1.1f));
        }


        void setDrive(float value) {
            setParam(DRIVE, clamp(value, 0.f, 1.1f));
        }


        float getDrive() {
            return getParam(DRIVE);
        }


        float getPeak() {
            return getParam(PEAK);
        }


        void setPeak(float value) {
            setParam(PEAK, clamp(value, 0.f, 1.1f));
        }


        void setIn(float value) {
            setInput(IN, value);
        }


        float getLPOut() {
            return getOutput(OUT);
        }


        float getType() {
            return getParam(TYPE);
        }


        void setType(float value) {
            setParam(TYPE, value);
        }

        void invalidate() override;
        void process() override;
    };


}