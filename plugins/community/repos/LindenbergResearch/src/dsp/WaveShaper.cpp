
#include "WaveShaper.hpp"

using namespace dsp;


double WaveShaper::getIn() const {
    return in;
}


void WaveShaper::setIn(double in) {
    WaveShaper::in = in;
}


double WaveShaper::getGain() const {
    return gain;
}


void WaveShaper::setGain(double gain) {
    WaveShaper::gain = gain;
}


double WaveShaper::getBias() const {
    return bias;
}


void WaveShaper::setBias(double bias) {
    WaveShaper::bias = bias;
}


double WaveShaper::getK() const {
    return k;
}


void WaveShaper::setK(double k) {
    WaveShaper::k = k;
}


double WaveShaper::getOut() const {
    return out;
}


void WaveShaper::setOut(double out) {
    WaveShaper::out = out;
}


const Vec &WaveShaper::getAmplitude() const {
    return amp;
}


void WaveShaper::process() {
    /* if no oversampling set up */
    if (rs->getFactor() == 1) {
        out = compute(in);
        //debug("%f", out);
        return;
    }

    rs->doUpsample(STD_CHANNEL, in);

    for (int i = 0; i < rs->getFactor(); i++) {
        double x = rs->getUpsampled(STD_CHANNEL)[i];
        rs->data[STD_CHANNEL][i] = compute(x);
    }

    out = rs->getDownsampled(STD_CHANNEL);
}


WaveShaper::WaveShaper(float sr) : DSPEffect(sr) {}







































