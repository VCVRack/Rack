#include "Overdrive.hpp"

using namespace dsp;


Overdrive::Overdrive(float sr) : WaveShaper(sr) {
    init();
    noise = new Noise;
    tanh1 = new HQTanh(sr, 1);
}


void Overdrive::init() {
    WaveShaper::rs = new Resampler<1>(4);
}


void Overdrive::process() {
    WaveShaper::process();
}


void Overdrive::invalidate() {}


double Overdrive::compute(double x) {
    double out, k;
    double in = clampd(x, -SHAPER_MAX_VOLTS, SHAPER_MAX_VOLTS);

    in *= clampd(gain, 0., 20.); // add gain
    in += clampd(bias * 2, -SHAPER_MAX_BIAS, SHAPER_MAX_BIAS); // add bias

    in *= OVERDRIVE_GAIN;

    in = tanh1->next(in * 1.5) * 1.5;

    double a = clampd(gain / 20, 0., .999999);

    k = 2 * a / (1 - a);
    in = (1 + k) * (in) / (1 + k * abs(in));

    in *= 1 / OVERDRIVE_GAIN * 0.3;
    // if (blockDC) in = dc->filter(in);

    out = in;

    return out;
}
