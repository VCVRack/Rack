#include "DSPMath.hpp"
#include "Oscillator.hpp"

using namespace dsp;


/**
 * @brief Set oscillator state back
 */
void BLITOscillator::reset() {
    freq = 0.f;
    pw = 1.f;
    phase = 0.f;
    incr = 0.f;
    shape = 1.f;
    detune = noise.nextFloat(0.32);
    drift = 0.f;
    warmup = 0.f;

    saw = 0.f;
    ramp = 0.f;
    pulse = 0.f;
    sine = 0.f;
    tri = 0.f;

    shape = 1.f;
    n = 0;

    _cv = 0.f;
    _oct = 0.f;

    _base = 1.f;
    _coeff = 1.f;
    _tune = 0.f;
    _biqufm = 0.f;

    /* force recalculation of variables */
    setFrequency(NOTE_C4);
}


/**
 * @brief Default constructor
 */
BLITOscillator::BLITOscillator() {
    reset();
}


/**
 * @brief Default destructor
 */
BLITOscillator::~BLITOscillator() {}


/**
 * @brief Get current frequency
 * @return
 */
float BLITOscillator::getFrequency() const {
    return freq;
}


/**
 * @brief Set frequency
 * @param freq
 */
void BLITOscillator::setFrequency(float freq) {
    /* just set if frequency differs from old value */
    if (BLITOscillator::freq != freq) {
        BLITOscillator::freq = freq;

        /* force recalculation of variables */
        invalidate();
    }
}


/**
 * @brief Get current pulse-width
 * @return
 */
float BLITOscillator::getPulseWidth() const {
    return pw;
}


/**
 * @brief Set current pulse-width
 * @param pw
 */
void dsp::BLITOscillator::setPulseWidth(float pw) {
    if (pw < 0.1f) {
        BLITOscillator::pw = 0.1f;
        return;
    }

    if (pw > 1.f) {
        BLITOscillator::pw = 1.f;
        return;
    }

    BLITOscillator::pw = pw;

    /* force recalculation of variables */
    invalidate();
}


/**
 * @brief Ramp waveform current
 * @return
 */
float BLITOscillator::getRampWave() const {
    return ramp;
}


/**
 * @brief Saw waveform current
 * @return
 */
float BLITOscillator::getSawWave() const {
    return saw;
}


/**
 * @brief Pulse waveform current
 * @return
 */
float BLITOscillator::getPulseWave() const {
    return pulse;
}


/**
 * @brief SawTri waveform current
 * @return
 */
float BLITOscillator::getSawTriWave() const {
    return sine;
}


/**
 * @brief Triangle waveform current
 * @return
 */
float BLITOscillator::getTriangleWave() const {
    return tri;
}


/**
 * @brief Process band-limited oscillator
 */
void dsp::BLITOscillator::proccess() {
    /* phase locked loop */
    phase = wrapTWOPI(incr + phase);

    /* pulse width */
    float w = pw * (float) M_PI;

    /* get impulse train */
    float blit1 = BLIT(n, phase);
    float blit2 = BLIT(n, wrapTWOPI(w + phase));

    /* feed integrator */
    int1.add(blit1, incr);
    int2.add(blit2, incr);

    /* integrator delta */
    float delta = int1.value - int2.value;

    /* 3rd integrator */
    float beta = int3.add(delta, incr) * 1.8f;

    /* compute RAMP waveform */
    ramp = int1.value * 0.5f;
    /* compute pulse waveform */
    pulse = delta;
    /* compute SAW waveform */
    saw = ramp * -1;

    /* compute triangle */
    tri = (float) M_PI / w * beta;
    /* compute sine */
    sine = fastSin(phase);

    //TODO: warmup oscillator with: y(x)=1-e^-(x/n) and slope

    saw *= 5;


/*    sine = shape2(shape, sine);
    tri = shape2(shape, tri);
    pulse = shape2(shape, pulse);*/

}


/**
 * @brief ReCompute basic parameter
 */
void BLITOscillator::invalidate() {
    incr = getPhaseIncrement(freq);
    n = (int) floorf(BLIT_HARMONICS / freq);
}


/**
 * @brief Get saturation
 * @return
 */
float BLITOscillator::getSaturate() const {
    return shape;
}


/**
 * @brief Set saturation
 * @param saturate
 */
void BLITOscillator::setShape(float saturate) {
    BLITOscillator::shape = saturate;
}


/**
 * @brief Translate from control voltage to frequency
 * @param cv ControlVoltage from MIDI2CV
 * @param fm Frequency modulation
 * @param oct Octave
 */
void dsp::BLITOscillator::updatePitch(float cv, float fm, float tune, float oct) {
    // CV is at 1V/OCt, C0 = 16.3516Hz, C4 = 261.626Hz
    // 10.3V = 20614.33hz

    /* optimize the usage of expensive exp function and other computations */
    float coeff = (_oct != oct) ? powf(2.f, oct) : _coeff;
    float base = (_cv != cv) ? powf(2.f, cv) : _base;
    float biqufm = (_tune != tune) ? quadraticBipolar(tune) : _biqufm;

    setFrequency((NOTE_C4 + biqufm) * base * coeff + detune + fm);

    /* save states */
    _cv = cv;
    _oct = oct;
    _base = base;
    _coeff = coeff;
    _tune = tune;
    _biqufm = biqufm;
}