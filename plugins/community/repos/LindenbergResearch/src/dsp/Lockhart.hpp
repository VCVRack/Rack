#pragma once

#include "WaveShaper.hpp"
#include "HQTrig.hpp"

// constants for Lockhart waveshaper model
#define LOCKHART_RL 7.5e3
#define LOCKHART_R 15e3
#define LOCKHART_VT 25.864e-3
#define LOCKHART_Is 10e-16
#define LOCKHART_THRESHOLD 10e-10

#define DCBLOCK_ALPHA 0.9999
namespace dsp {

    /**
     * @brief Represents one stage of the Lockhart Wavefolder
     */
    struct LockhartWFStage {
    private:

        double fn1, xn1;
        double a, b, d;

    public:

        LockhartWFStage();

        double compute(double x);
    };


    /**
     * Lockhart Wavefolder class
     */
    struct LockhartWavefolder : WaveShaper {

    private:
        LockhartWFStage lh1, lh2, lh3, lh4;
        DCBlocker *dc = new DCBlocker(DCBLOCK_ALPHA);
        HQTanh *tanh1;
        bool blockDC = false;


    public:
        explicit LockhartWavefolder(float sr);

        void init() override;
        void invalidate() override;
        void process() override;
        double compute(double x) override;


        bool isBlockDC() const;
        void setBlockDC(bool blockDC);

    };

}
