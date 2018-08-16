#pragma once

#include "DSPMath.hpp"
#include "DSPSystem.hpp"

#define LFO_SCALE 25.f
#define TUNE_SCALE 17.3f
#define LFO_MODE -4
#define CV_BOUNDS 10.f
#define DETUNE_AMOUNT  2.0f
#define DRIFT_AMOUNT 0.8f
#define DRIFT_FREQ 0.0005f
#define DRIFT_VARIANZ 0.004

namespace dsp {

    /**
     * @brief Simple and fast LFO
     */
    struct DSPSineLFO : DSPSystem<0, 1, 1> {
        enum Outputs {
            SINE
        };

        enum Params {
            FREQ
        };

    private:
        float frac, phase;

    public:

        DSPSineLFO(float sr) : DSPSystem(sr) {
            setFrequency(1.f);
        }

        void setFrequency(float freq) {
            setParam(FREQ, freq, true);
        }


        float getFrequency() {
            return getParam(FREQ);
        }


        void invalidate() override {
            frac = TWOPI / sr * param[FREQ].value;
        }


        void process() override {
            phase = wrapTWOPI(phase + frac);
            output[SINE].value = fastSin(phase);
        }


        float getSine() {
            return output[SINE].value;
        }


        void setPhase(float phase) {
            DSPSineLFO::phase = phase;
        }


        void reset() {
            phase = 0;
        }

    };


    /**
     * DSP model of a leaky integrator
     */
    struct DSPIntegrator : DSPSystem<1, 1, 1> {
        enum Inputs {
            IN
        };

        enum Outputs {
            OUT
        };

        enum Params {
            LEAK
        };

    private:
        float z = 0;

    public:
        void process() override {
            output[OUT].value = z;
            z = input[IN].value + (param[LEAK].value * z);
        }


        /**
         * @brief Add a value to integrator and push output
         * @param x Input value
         * @return Current of integrator
         */
        float add(float x, float leak) {
            param[LEAK].value = 0.999;
            input[IN].value = x;
            process();

            return output[OUT].value;
        }


        /**
         * @brief Returns the current integrator state
         * @return
         */
        float value() {
            return z;
        }
    };


    struct DSPBLOscillator : DSPSystem<4, 6, 10> {
        /**
         * Bandwidth-limited threshold in hz.
         * Should be at least SR/2 !
         * */
        static constexpr float BLIT_HARMONICS = 18000.f;
        static constexpr float NOTE_C4 = 261.626f;

        enum Inputs {
            VOCT1, VOCT2,
            FM_CV,
            TUNE,
            OCTAVE
        };

        enum Outputs {
            SAW,
            PULSE,
            SINE,
            TRI,
            NOISE
        };

        enum Params {
            FREQUENCY,
            PULSEWIDTH
        };

    private:
        float phase;     // current phase
        float incr;      // current phase increment for PLL
        float detune;    // analogue detune
        float drift;     // oscillator drift
        float warmup;    // oscillator warmup detune
        float warmupTau; // time factor for warmup detune
        int tick;
        int n;
        bool lfoMode;    // LFO mode?
        Noise noise;     // randomizer

        Integrator int1;
        Integrator int2;
        Integrator int3;

        DSPSineLFO *lfo;


        void reset();

        /* saved frequency states */
        float _cv, _oct, _base, _coeff, _tune, _biqufm;


    public:
        explicit DSPBLOscillator(float sr);

        void updatePitch();

        void setFrequency(float frq);


        void setInputs(float voct1, float voct2, float fm, float tune, float oct);


        float getFrequency() { return param[FREQUENCY].value; }


        bool isLFO() {
            return lfoMode;
        }


        void setPulseWidth(float width);


        float getSawWave() {
            return getOutput(SAW);
        }


        float getPulseWave() {
            return getOutput(PULSE);
        }


        float getSineWave() {
            return getOutput(SINE);
        }


        float getTriWave() {
            return getOutput(TRI);
        }


        float getNoise() {
            return getOutput(NOISE);
        }


        void updateSampleRate(float sr) override;

        void invalidate() override;
        void process() override;
    };


}