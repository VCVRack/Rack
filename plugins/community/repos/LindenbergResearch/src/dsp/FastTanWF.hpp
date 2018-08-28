#pragma once

#include "WaveShaper.hpp"

#define FASTTAN_GAIN 0.05

namespace dsp {

struct FastTan : WaveShaper {

    Noise *noise;

public:

    explicit FastTan(float sr);

    void init() override;
    void invalidate() override;
    void process() override;
    double compute(double x) override;

};

}