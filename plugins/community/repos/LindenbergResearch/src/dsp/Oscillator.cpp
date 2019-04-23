#include "DSPMath.hpp"
#include "Oscillator.hpp"

using namespace dsp;


/**
 * @brief Construct a Oscillator
 * @param sr SampleRate
 */
DSPBLOscillator::DSPBLOscillator(float sr) : DSPSystem(sr) {
    lfo = new DSPSineLFO(sr);
    reset();
}


/**
 * @brief Trigger recalculation of internal state
 */
void DSPBLOscillator::invalidate() {
    incr = getPhaseIncrement(param[FREQUENCY].value);
    n = floorf(sr * 0.5 / param[FREQUENCY].value);
}


/**
 * @brief Process one sample
 */
void DSPBLOscillator::process() {
    updatePitch();

    /* phase locked loop */
    phase = wrapTWOPI(incr + phase);

    /* pulse width */
    float w = param[PULSEWIDTH].value * PI;

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
    float ramp = int1.value * 0.5f;

    /* compute pulse waveform */
    output[PULSE].value = delta * 2.f;

    /* compute SAW waveform */
    output[SAW].value = ramp * -5;

    /* compute triangle */
    output[TRI].value = beta * 5.f;

    /* compute sine */
    output[SINE].value = fastSin(phase) * 5.f;

    /* compute noise: act as S&H in LFO mode, update next random only every cycle */
    if (!lfoMode || phase - incr <= -M_PI)
        output[NOISE].value = noise.nextFloat(10.f) - 5.f;

}


void DSPBLOscillator::reset() {
    param[FREQUENCY].value = 0.f;
    param[PULSEWIDTH].value = 1.f;
    phase = 0.f;
    incr = 0.f;
    detune = noise.nextFloat(DETUNE_AMOUNT);
    drift = 0.f;
    warmup = 0.f;
    warmupTau = sr * 1.5f;
    tick = round(sr * 0.7f);

    lfo->reset();
    lfo->setPhase(noise.nextFloat(TWOPI));
    lfo->setFrequency(DRIFT_FREQ + noise.nextFloat(DRIFT_VARIANZ));

    n = 0;

    _cv = 0.f;
    _oct = 0.f;

    _base = 1.f;
    _coeff = 1.f;
    _tune = 0.f;
    _biqufm = 0.f;

    /* force recalculation of variables */
    setParam(FREQUENCY, NOTE_C4 + detune, true);
}


/**
 * @brief Constructs the correct pitch in Hz out of all parameters
 * @param cv V/OCT CVs
 * @param fm Frequency modulation CVs
 * @param tune Tune knob value
 * @param oct Octave knob value
 */
void DSPBLOscillator::updatePitch() {
    // CV is at 1V/OCt, C0 = 16.3516Hz, C4 = 261.626Hz
    // 10.3V = 20614.33hz

    // give it 30s to warmup
    if (tick++ < sr * 30) {
        if (tick < sr * 1.8f) {
            tick += 6; // accelerated detune
            warmup = 1 - powf((float) M_E, -(tick / warmupTau));
        } else
            warmup = 1 - powf((float) M_E, -(tick / warmupTau));
    }

    lfo->process();
    drift = lfo->getSine() * DRIFT_AMOUNT;

    float cv = input[VOCT1].value + input[VOCT2].value;
    float fm;
    float tune;
    float oct;

    if (lfoMode) {
        /* convert knob value to unipolar */
        fm = input[FM_CV].value;
        tune = quadraticBipolar((input[TUNE].value + 1) / 2);
        tune *= LFO_SCALE;
        fm *= LFO_SCALE;
        oct = -8;
    } else {
        fm = input[FM_CV].value * TUNE_SCALE;
        tune = input[TUNE].value * TUNE_SCALE;
        oct = input[OCTAVE].value;
    }

    /* optimize the usage of expensive exp function and other computations */
    float coeff = (_oct != oct) ? powf(2.f, oct) : _coeff;
    float base = (_cv != cv) ? powf(2.f, cv) : _base;
    float biqufm = (_tune != tune + fm) ? quadraticBipolar(tune + fm) : _biqufm;

    if (lfoMode)
        setFrequency(tune + fm);
    else
        setFrequency((NOTE_C4 + drift + detune + biqufm) * base * coeff * warmup);

    /* save states */
    _cv = cv;
    _oct = oct;
    _base = base;
    _coeff = coeff;
    _tune = tune + fm;
    _biqufm = biqufm;
}


void DSPBLOscillator::setFrequency(float frq) {
    setParam(FREQUENCY, clamp(frq, 0.00001f, 18000.f), true);
}


void DSPBLOscillator::setPulseWidth(float width) {
    setParam(PULSEWIDTH, width, true);
}


void DSPBLOscillator::setInputs(float voct1, float voct2, float fm, float tune, float oct) {
    setInput(VOCT1, voct1);
    setInput(VOCT2, voct2);
    setInput(FM_CV, fm);
    setInput(TUNE, tune);
    setInput(OCTAVE, oct);

    /* check for lowest value on toggle knob */
    lfoMode = oct == LFO_MODE;
}


/**
 * @brief Pass changed samplerate to LFO
 * @param sr
 */
void DSPBLOscillator::updateSampleRate(float sr) {
    DSPSystem::updateSampleRate(sr);
    lfo->updateSampleRate(sr);
}

