#include "SergeWavefolder.hpp"

using namespace dsp;


double SergeWFStage::compute(double x) {
    double out;
    double l, u, ln, fn, xn;

    l = sign(x);
    u = (SERGE_R1 * SERGE_IS) / (SERGE_N * SERGE_VT) * pow(M_E, (l * x) / (SERGE_N * SERGE_VT));
    ln = lambert_W_Fritsch(u);
    fn = x * x / 2 - SERGE_VT * SERGE_N * SERGE_N * SERGE_VT * (ln * (ln + 2));

    // Check for ill-conditioning
    if (abs(x - xn1) < SERGE_THRESHOLD) {
        // Compute Averaged Wavefolder Output
        xn = 0.5 * (x + xn1);
        u = (SERGE_R1 * SERGE_IS) / (SERGE_N * SERGE_VT) * pow(M_E, (l * xn) / (SERGE_VT * SERGE_N));
        ln = lambert_W_Fritsch(u);
        out = xn - 2 * l * SERGE_N * SERGE_VT * ln;
    } else {
        // Apply AA Form
        out = (fn - fn1) / (x - xn1);
    }

    fn1 = fn;
    xn1 = x;

    return out;
}


SergeWFStage::SergeWFStage() {
    fn1 = 0;
    xn1 = 0;
}


SergeWavefolder::SergeWavefolder(float sr) : WaveShaper(sr) {
    init();
    tanh1 = new HQTanh(sr, 4);
}


void SergeWavefolder::init() {
    dsp::WaveShaper::rs = new dsp::Resampler<1>(1);
}


void SergeWavefolder::process() {
    WaveShaper::process();
}


double SergeWavefolder::compute(double x) {
    double out;
    double in = (x / 8. + bias) * gain;

    // in *= 0.5;

    in = sg1.compute(in);
    in = sg2.compute(in);
    in = sg3.compute(in);
    in = sg4.compute(in);
    in = sg5.compute(in);
    in = sg6.compute(in);

    in *= 4.f;

    out = in;// tanh1->next(in / 2.);

    return out;
}

