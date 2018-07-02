#pragma once

#include "DSPMath.hpp"
#include "DSPSystem.hpp"

#define BLIT_HARMONICS 21000.f
#define NOTE_C4 261.626f

namespace dsp {

    struct DSPIntegrator : DSPSystem<1, 1, 1> {
        enum Inputs {
            IN
        };

        enum Outputs {
            OUT
        };

        enum Params {
            D
        };

        float d = 0.25;
        float x = 0;


    };


/**
 * @brief Oscillator base class
 */
    struct BLITOscillator {

    public:
        enum SIGNAL {
            SAW,
            PULSE,
            SINE,
            TRI
        };


        float freq;      // oscillator frequency
        float pw;        // pulse-width value
        float phase;     // current phase
        float incr;      // current phase increment for PLL
        float detune;    // analogue detune
        float drift;     // oscillator drift
        float warmup;    // oscillator warmup detune
        Noise noise;     // randomizer

        float shape;
        int n;

        /* currents of waveforms */
        float ramp;
        float saw;
        float pulse;
        float sine;
        float tri;

        /* saved frequency states */
        float _cv, _oct, _base, _coeff, _tune, _biqufm;

        /* leaky integrators */
        Integrator int1;
        Integrator int2;
        Integrator int3;

        BLITOscillator();
        ~BLITOscillator();

        /**
         * @brief Proccess next sample for output
         */
        void proccess();


        /**
         * @brief ReCompute states on change
         */
        void invalidate();


        /**
         * @brief Reset oscillator
         */
        void reset();


        void updatePitch(float cv, float fm, float tune, float oct);

        /* common getter and setter */
        float getFrequency() const;
        void setFrequency(float freq);
        float getPulseWidth() const;
        void setPulseWidth(float pw);

        float getRampWave() const;
        float getSawWave() const;
        float getPulseWave() const;
        float getSawTriWave() const;
        float getTriangleWave() const;
        float getSaturate() const;
        void setShape(float saturate);
    };

}