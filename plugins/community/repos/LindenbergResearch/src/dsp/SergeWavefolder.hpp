#pragma once


#include "WaveShaper.hpp"
#include "HQTrig.hpp"

#define SERGE_R1 33e3
#define SERGE_IS 2.52e-9
#define SERGE_VT 25.864e-3
#define SERGE_N 1.752

#define SERGE_THRESHOLD 10e-10

namespace dsp {

    struct SergeWFStage {
    private:
        double fn1, xn1;

    public:
        SergeWFStage();
        double compute(double x);
    };


    struct SergeWavefolder : WaveShaper {

    private:
        SergeWFStage sg1, sg2, sg3, sg4, sg5, sg6;
        //   DCBlocker *dc = new DCBlocker(DCBLOCK_ALPHA);
        HQTanh *tanh1;
        bool blockDC = false;


    public:
        explicit SergeWavefolder(float sr);

        void init() override;
        void process() override;
        double compute(double x) override;

    };


}