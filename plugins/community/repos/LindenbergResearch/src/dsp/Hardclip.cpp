#include "Hardclip.hpp"

using namespace dsp;


Hardclip::Hardclip(float sr) : WaveShaper(sr) {
    init();
    noise = new Noise;
    hqclip = new HQClip(sr, 4);
}


void Hardclip::init() {
    WaveShaper::rs = new Resampler<1>(1);
}


void Hardclip::process() {
    WaveShaper::process();
}


void Hardclip::invalidate() {}


double Hardclip::compute(double x) {
    double out;
    double in = clampd(x, -SHAPER_MAX_VOLTS, SHAPER_MAX_VOLTS);

    in *= clampd(gain, 0., 20.); // add gain
    in += clampd(bias * 2, -SHAPER_MAX_BIAS, SHAPER_MAX_BIAS); // add bias

    in *= HARDCLIP_GAIN;

    in = hqclip->next(in);

    in *= 1 / HARDCLIP_GAIN * 0.3;
    if (blockDC) in = dc->filter(in);

    out = in + noise->nextFloat(HARDCLIP_NOISE);

    return out;
}
