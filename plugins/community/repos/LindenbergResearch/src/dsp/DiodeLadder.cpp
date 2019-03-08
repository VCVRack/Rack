#include "DiodeLadder.hpp"

using namespace dsp;


DiodeLadderStage::DiodeLadderStage(float sr) : DSPEffect(sr) {

}


void DiodeLadderStage::init() {
    alpha = 1.f;
    beta = -1.f;
    gamma = 1.f;
    delta = 0.f;
    epsilon = 1.f;
    feedback = 0.f;
    gain = 1.f;


    resetZ1();
}


void DiodeLadderStage::invalidate() {
    DSPEffect::invalidate();
}


void DiodeLadderStage::process() {
    float x = (in * gamma + feedback + epsilon * getFeedbackOutput());
    float vn = (gain * x - z1) * alpha;

    out = vn + z1;

    z1 = vn + out;
}


float DiodeLadderStage::getFeedbackOutput() {
    return (z1 + feedback * delta) * beta;
}


DiodeLadderFilter::DiodeLadderFilter(float sr) : DSPEffect(sr) {
    lpf1 = new DiodeLadderStage(sr);
    lpf2 = new DiodeLadderStage(sr);
    lpf3 = new DiodeLadderStage(sr);
    lpf4 = new DiodeLadderStage(sr);

    rs = new Resampler<1>(OVERSAMPLE, 4);

    gamma = 0.f;
    k = 0.f;
    saturation = 1.f;

    sg1 = 0.f;
    sg2 = 0.f;
    sg3 = 0.f;
    sg4 = 0.f;

    lpf1->gain = 1.f;
    lpf2->gain = 0.5f;
    lpf3->gain = 0.5f;
    lpf4->gain = 0.5f;

    lpf4->gamma = 1.f;
    lpf4->delta = 0.f;
    lpf4->epsilon = 0.f;
    lpf4->setFeedback(0.f);
}


void DiodeLadderFilter::init() {
    DSPEffect::init();
    reset();
    invalidate();
}


void DiodeLadderFilter::invalidate() {
    float G1, G2, G3, G4;

    float SR = low ? sr : sr * OVERSAMPLE;

    freqHz = MAX_FREQUENCY / 1000.f * powf(1000.f, fc);
    // freqHz = 40.f * powf(500.f, fc);

    float wd = TWOPI * freqHz;
    float T = 1 / SR;
    float wa = (2 / T) * tanf(wd * T / 2);
    float g = wa * T / 2;

    G4 = 0.5f * g / (1.0f + g);
    G3 = 0.5f * g / (1.0f + g - 0.5f * g * G4);
    G2 = 0.5f * g / (1.0f + g - 0.5f * g * G3);
    G1 = g / (1.0f + g - g * G2);

    gamma = G4 * G3 * G2 * G1;

    sg1 = G4 * G3 * G2;
    sg2 = G4 * G3;
    sg3 = G4;
    sg4 = 1.0f;

    lpf1->alpha = g / (1.0f + g);
    lpf2->alpha = g / (1.0f + g);
    lpf3->alpha = g / (1.0f + g);
    lpf4->alpha = g / (1.0f + g);

    lpf1->beta = 1.0f / (1.0f + g - g * G2);
    lpf2->beta = 1.0f / (1.0f + g - 0.5f * g * G3);
    lpf3->beta = 1.0f / (1.0f + g - 0.5f * g * G4);
    lpf4->beta = 1.0f / (1.0f + g);

    lpf1->gamma = 1.0f + G1 * G2;
    lpf2->gamma = 1.0f + G2 * G3;
    lpf3->gamma = 1.0f + G3 * G4;

    lpf1->delta = g;
    lpf2->delta = 0.5f * g;
    lpf3->delta = 0.5f * g;

    lpf1->epsilon = G2;
    lpf2->epsilon = G3;
    lpf3->epsilon = G4;
}


void DiodeLadderFilter::process() {
    if (low) {
        process1();
    } else {
        process2();
    }
}


void DiodeLadderFilter::process1() {
    lpf3->setFeedback(lpf4->getFeedbackOutput());
    lpf2->setFeedback(lpf3->getFeedbackOutput());
    lpf1->setFeedback(lpf2->getFeedbackOutput());

    float sigma = sg1 * lpf1->getFeedbackOutput() +
                  sg2 * lpf2->getFeedbackOutput() +
                  sg3 * lpf3->getFeedbackOutput() +
                  sg4 * lpf4->getFeedbackOutput();

    float y = (1.0f / fastatan(saturation)) * fastatan(saturation * in);

    y += noise.nextFloat(NOISE_GAIN);

    float u = (y - k * sigma) / (1 + k * gamma);

    u = fastatan(u / FEEDBACK_LIMITER_GAIN) * FEEDBACK_LIMITER_GAIN; // limit feedback gain of resonance

    lpf1->in = u;
    lpf1->process();

    lpf2->in = lpf1->out;
    lpf2->process();

    lpf3->in = lpf2->out;
    lpf3->process();

    lpf4->in = lpf3->out;
    lpf4->process();

    out2 = tanh(u - lpf4->out);
    out = tanh(lpf4->out);
}


void DiodeLadderFilter::process2() {
    rs->doUpsample(IN, in);
    for (int i = 0; i < rs->getFactor(); i++) {
        in = (float) rs->getUpsampled(IN)[i];

        process1();

        rs->data[IN][i] = out;
    }

    out = (float) rs->getDownsampled(IN);;
}


void DiodeLadderFilter::setSamplerate(float sr) {
    DSPEffect::setSamplerate(sr);

    /* set samplerate for all submodules */
    lpf1->setSamplerate(sr);
    lpf2->setSamplerate(sr);
    lpf3->setSamplerate(sr);
    lpf4->setSamplerate(sr);
}


void DiodeLadderFilter::setFrequency(float fc) {
    DiodeLadderFilter::fc = fc;
}


void DiodeLadderFilter::setResonance(float k) {
    DiodeLadderFilter::k = k;
}


void DiodeLadderFilter::setIn(float in) {
    DiodeLadderFilter::in = in;
}


float DiodeLadderFilter::getOut() const {
    return out;
}


void DiodeLadderFilter::setSaturation(float saturation) {
    DiodeLadderFilter::saturation = saturation;
}


float DiodeLadderFilter::getOut2() const {
    return out2;
}
