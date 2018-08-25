#include "RShaper.hpp"

using namespace dsp;


ReShaper::ReShaper(float sr) : WaveShaper(sr) {
    init();
    noise = new Noise;
}


void ReShaper::init() {
    WaveShaper::rs = new Resampler<1>(8, 16);
}


void ReShaper::process() {
    WaveShaper::process();
}


void ReShaper::invalidate() {}


double ReShaper::compute(double x) {
    double out;
    double a = gain * 2.5;
    double in = clampd(x, -SHAPER_MAX_VOLTS, SHAPER_MAX_VOLTS);

    in += clampd(bias * 0.5, -SHAPER_MAX_BIAS / 4., SHAPER_MAX_BIAS / .4); // add bias

    in *= RSHAPER_GAIN;

    in = in * (fabs(in) + a) / (in * in + (a - 1) * fabs(in) + 1);

    in *= 1 / RSHAPER_GAIN * 0.5;
    if (blockDC) in = dc->filter(in);

    out = in;

    return out;
}
