#include "Lockhart.hpp"

using namespace dsp;


double LockhartWFStage::compute(double x) {
    double out;
    double l, u, ln, fn, xn;

    // Compute Antiderivative
    l = sign(x);
    u = d * pow(M_E, l * b * x);
    ln = lambert_W_Fritsch(u);
    fn = (0.5 * LOCKHART_VT / b) * (ln * (ln + 2.)) - 0.5 * a * x * x;

    // Check for ill-conditioning
    if (abs(x - xn1) < LOCKHART_THRESHOLD) {
        // Compute Averaged Wavefolder Output
        xn = 0.5 * (x + xn1);
        u = d * pow(M_E, l * b * xn);
        ln = lambert_W_Fritsch(u);
        out = l * LOCKHART_VT * ln - a * xn;

    } else {
        // Apply AA Form
        out = (fn - fn1) / (x - xn1);
    }

    fn1 = fn;
    xn1 = x;

    return out;
}


LockhartWFStage::LockhartWFStage() {
    fn1 = 0;
    xn1 = 0;

    a = 2. * LOCKHART_RL / LOCKHART_R;
    b = (LOCKHART_R + 2. * LOCKHART_RL) / (LOCKHART_VT * LOCKHART_R);
    d = (LOCKHART_RL * LOCKHART_Is) / LOCKHART_VT;
}


void LockhartWavefolder::init() {
    dsp::WaveShaper::rs = new dsp::Resampler<1>(1);
}


void LockhartWavefolder::invalidate() {
}


void LockhartWavefolder::process() {
    WaveShaper::process();
}


double LockhartWavefolder::compute(double x) {
    double out;
    double in = (x / 8. + bias) * gain;

    in *= 0.5;

    in = lh1.compute(in);
    in = lh2.compute(in);
    in = lh3.compute(in);
    in = lh4.compute(in);

    in *= 2.f;

    if (blockDC) in = dc->filter(in);

    out = tanh1->next(in / 2.);

    return out * 20 * 2;
}


LockhartWavefolder::LockhartWavefolder(float sr) : WaveShaper(sr) {
    init();
    tanh1 = new HQTanh(sr, 4);
}


bool LockhartWavefolder::isBlockDC() const {
    return blockDC;
}


void LockhartWavefolder::setBlockDC(bool blockDC) {
    LockhartWavefolder::blockDC = blockDC;
}
