#include "FastTanWF.hpp"

using namespace dsp;


FastTan::FastTan(float sr) : WaveShaper(sr) {
    init();
    noise = new Noise;
}


void FastTan::init() {
    WaveShaper::rs = new Resampler<1>(8, 16);
}


void FastTan::process() {
    WaveShaper::process();
}


void FastTan::invalidate() {}


double FastTan::compute(double x) {
    double out;
    double in = clampd(x, -SHAPER_MAX_VOLTS, SHAPER_MAX_VOLTS);

    in *= clampd(gain, 0., 20.); // add gain
    in += clampd(bias * 2, -SHAPER_MAX_BIAS, SHAPER_MAX_BIAS); // add bias

    in *= FASTTAN_GAIN;

    in = fastatan(in * 10) / 10;

    in *= 1 / FASTTAN_GAIN * (1 + gain / 15);
    if (blockDC) in = dc->filter(in);

    out = in;

    return out;
}
