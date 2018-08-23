#include "Saturator.hpp"

using namespace dsp;


Saturator::Saturator(float sr) : WaveShaper(sr) {
    init();
    noise = new Noise;
    tanh1 = new HQTanh(sr, 4);
}


void Saturator::init() {
    WaveShaper::rs = new Resampler<1>(1);
}


void Saturator::process() {
    WaveShaper::process();
}


void Saturator::invalidate() {}


double Saturator::compute(double x) {
    double out;
    double in = clampd(x, -SHAPER_MAX_VOLTS, SHAPER_MAX_VOLTS);

    in *= clampd(gain, 0., 20.); // add gainBtn
    in += clampd(bias * 2, -12., 12.); // add biasBtn

    in *= SATURATOR_GAIN;

    in = tanh1->next(in);

    in *= 1 / SATURATOR_GAIN * 0.3;
    if (blockDC) in = dc->filter(in);

    out = in + noise->nextFloat(SATURATOR_NOISE);

    return out;
}
