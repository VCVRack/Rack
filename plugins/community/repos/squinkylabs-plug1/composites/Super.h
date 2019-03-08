
#pragma once

#include "GateTrigger.h"
#include "IIRDecimator.h"
#include "NonUniformLookupTable.h"
#include "ObjectCache.h"
#include "StateVariable4PHP.h"

class SawtoothDetuneCurve
{
public:
    /**
    * @param depth is "detune" knob. 0..1
    * returns a number such that freq = detuneFactor * initialFreq
    */
    float getDetuneFactor(float depth)
    {
        return NonUniformLookupTable<float>::lookup(table, depth);
    }

    SawtoothDetuneCurve()
    {
        // this data is pretty regular - could use uniform table
        using T = NonUniformLookupTable<float>;
        T::addPoint(table, 0, 0);
        T::addPoint(table, .0551f, .00967f);
        T::addPoint(table, .118f, .022f);
        T::addPoint(table, .181f, .04f);
        T::addPoint(table, .244f, .0467f);
        T::addPoint(table, .307f, .059f);

        T::addPoint(table, .37f, .0714f);
        T::addPoint(table, .433f, .0838f);
        T::addPoint(table, .496f, .0967f);
        T::addPoint(table, .559f, .121f);
        T::addPoint(table, .622f, .147f);
        T::addPoint(table, .748f, .243f);
        T::addPoint(table, .811f, .293f);
        T::addPoint(table, .874f, .343f);
        T::addPoint(table, .937f, .392f);
        T::addPoint(table, 1, 1);
        NonUniformLookupTable<float>::finalize(table);
    }
private:
    NonUniformLookupTableParams<float> table;
};


/**
 * orig CPU = 39
 * sub sample => 16
 * beta1 => 16.1
        17.7 if change pitch every 16 samples.

 * beta 3: fix instability
 * made semitone and fine ranges sane.
 */
template <class TBase>
class Super : public TBase
{
public:

    Super(struct Module * module) : TBase(module), gateTrigger(true)
    {
        init();
    }
    Super() : TBase(), gateTrigger(true)
    {
        init();
    }

    /**
    * re-calculate everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        OCTAVE_PARAM,
        SEMI_PARAM,
        FINE_PARAM,
        DETUNE_PARAM,
        DETUNE_TRIM_PARAM,
        MIX_PARAM,
        MIX_TRIM_PARAM,
        FM_PARAM,
        CLEAN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        TRIGGER_INPUT,
        DETUNE_INPUT,
        MIX_INPUT,
        FM_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:
    static const unsigned int MAX_OVERSAMPLE = 16;
    static const int numSaws = 7;

    float phase[numSaws] = {0};
    float phaseInc[numSaws] = {0};
    float globalPhaseInc = 0;

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    // knob, cv, trim -> 0..1
    AudioMath::ScaleFun<float> scaleDetune;

    float runSaws();
    void updatePhaseInc();
    void updateAudioClassic();
    void updateAudioClean();
    void updateTrigger();
    void updateMix();

    int getOversampleRate();

    AudioMath::RandomUniformFunc random = AudioMath::random();

    int inputSubSampleCounter = 1;
    const static int inputSubSample = 4;    // only look at knob/cv every 4

    // TODO: make static
    float const detuneFactors[numSaws] = {
        .89f,
        .94f,
        .98f,
        1.f,
        1.02f,
        1.06f,
        1.107f
    };

    void updateHPFilters();

    SawtoothDetuneCurve detuneCurve;
    GateTrigger gateTrigger;
    float gainCenter = 0;
    float gainSides = 0;

    StateVariable4PHP hpf;

    float buffer[MAX_OVERSAMPLE] = {};
    IIRDecimator decimator;
};

template <class TBase>
inline void Super<TBase>::init()
{
    scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);

    const int rate = getOversampleRate();
    const int decimateDiv = std::max(rate, (int) MAX_OVERSAMPLE);
    decimator.setup(decimateDiv);
}

template <class TBase>
inline int Super<TBase>::getOversampleRate()
{
    int rate = 1;
    const int setting = (int) std::round(TBase::params[CLEAN_PARAM].value);
    switch (setting) {
        case 0:
            rate = 1;
            break;
        case 1:
            rate = 4;
            break;
        case 2:
            rate = 16;
            break;
        default:
            assert(false);
    }
    assert(rate <= (int) MAX_OVERSAMPLE);
    return rate;
}

template <class TBase>
inline void Super<TBase>::updatePhaseInc()
{
    const float cv = TBase::inputs[CV_INPUT].value;

    const float finePitch = TBase::params[FINE_PARAM].value / 12.0f;
    const float semiPitch = TBase::params[SEMI_PARAM].value / 12.0f;

    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value) +
        semiPitch +
        finePitch;

    pitch += cv;

    const float fm = TBase::inputs[FM_INPUT].value;
    const float fmDepth = AudioMath::quadraticBipolar(TBase::params[FM_PARAM].value);

    pitch += (fmDepth * fm);

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = TBase::engineGetSampleTime() * freq;

    const float rawDetuneValue = scaleDetune(
        TBase::inputs[DETUNE_INPUT].value,
        TBase::params[DETUNE_PARAM].value,
        TBase::params[DETUNE_TRIM_PARAM].value);

    const float detuneInput = detuneCurve.getDetuneFactor(rawDetuneValue);


   // const bool classic = TBase::params[CLEAN_PARAM].value < .5f;
    const int oversampleRate = getOversampleRate();

    for (int i = 0; i < numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * detuneInput;
        detune += 1;
        float phaseIncI = globalPhaseInc * detune;
        phaseIncI = std::min(phaseIncI, .4f);         // limit so saws don't go crazy
        if (oversampleRate > 1) {
            phaseIncI /= oversampleRate;
        }
        phaseInc[i] = phaseIncI;
    }
}


template <class TBase>
inline float Super<TBase>::runSaws()
{
    float mix = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        const float gain = (i == numSaws / 2) ? gainCenter : gainSides;
      //  mix += phase[i] * gain;
        mix += (phase[i] - .5f) * gain;        // experiment to get rid of DC
    }

    mix *= 4.5;       // too low 2 too high 10
    return mix;
}

template <class TBase>
inline void Super<TBase>::updateAudioClassic()
{
    const float mix = runSaws();
    const float output = hpf.run(mix);
    TBase::outputs[MAIN_OUTPUT].value = output;
}

template <class TBase>
inline void Super<TBase>::updateAudioClean()
{
    const int bufferSize = getOversampleRate();
    decimator.setup(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        const float mix = runSaws();
        buffer[i] = mix;
    }
    //const float output = hpf.run(mix);
    const float output = decimator.process(buffer);
    TBase::outputs[MAIN_OUTPUT].value = output;
}
template <class TBase>
inline void Super<TBase>::updateHPFilters()
{
    const float filterCutoff = std::min(globalPhaseInc, .1f);
    hpf.setCutoff(filterCutoff);
}

template <class TBase>
inline void Super<TBase>::step()
{
    updateTrigger();
    if (--inputSubSampleCounter <= 0) {
        inputSubSampleCounter = inputSubSample;
        updatePhaseInc();
        updateHPFilters();
        updateMix();
    }
   // const bool classic = TBase::params[CLEAN_PARAM].value < .5f;
    int rate = getOversampleRate();
    if (rate == 1) {
        updateAudioClassic();
    } else {
        updateAudioClean();
    }
}

template <class TBase>
inline void Super<TBase>::updateTrigger()
{
    gateTrigger.go(TBase::inputs[TRIGGER_INPUT].value);
    if (gateTrigger.trigger()) {
        for (int i = 0; i < numSaws; ++i) {
            phase[i] = this->random();
        }
    }
}

template <class TBase>
inline void Super<TBase>::updateMix()
{
    const float rawMixValue = scaleDetune(
        TBase::inputs[MIX_INPUT].value,
        TBase::params[MIX_PARAM].value,
        TBase::params[MIX_TRIM_PARAM].value);

    gainCenter = -0.55366f * rawMixValue + 0.99785f;

    gainSides = -0.73764f * rawMixValue * rawMixValue +
        1.2841f * rawMixValue + 0.044372f;
}