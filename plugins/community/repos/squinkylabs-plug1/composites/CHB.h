
#pragma once

#include <algorithm>

#include "AudioMath.h"
#include "IComposite.h"
#include "LookupTableFactory.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "poly.h"
#include "SinOscillator.h"

using Osc = SinOscillator<float, true>;

#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif



template <class TBase>
class CHBDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * Composite for Chebyshev module.
 *
 * Performance measure for 1.0 = 42.44
 * reduced polynomial order to what we actually use (10), perf = 39.5
 */
template <class TBase>
class CHB : public TBase
{
public:
    CHB(struct Module * module) : TBase(module)
    {
        init();
    }
    CHB() : TBase()
    {
        init();
    }

    enum ParamIds
    {
        PARAM_TUNE,
        PARAM_OCTAVE,
        PARAM_EXTGAIN,
        PARAM_PITCH_MOD_TRIM,
        PARAM_LINEAR_FM_TRIM,
        PARAM_EXTGAIN_TRIM,
        PARAM_FOLD,
        PARAM_SLOPE,
        PARAM_MAG_EVEN,
        PARAM_MAG_ODD,
        PARAM_H0,
        PARAM_H1,
        PARAM_H2,
        PARAM_H3,
        PARAM_H4,
        PARAM_H5,
        PARAM_H6,
        PARAM_H7,
        PARAM_H8,
        PARAM_H9,       // up to here is Ver 1.0
        PARAM_EXPAND,
        PARAM_RISE,
        PARAM_FALL,
        PARAM_EVEN_TRIM,
        PARAM_ODD_TRIM,
        PARAM_SLOPE_TRIM,
        PARAM_SEMIS,
        NUM_PARAMS
    };
    const int numHarmonics = 1 + PARAM_H9 - PARAM_H0;

    enum InputIds
    {
        CV_INPUT,
        PITCH_MOD_INPUT,
        LINEAR_FM_INPUT,
        ENV_INPUT,
        GAIN_INPUT,
        AUDIO_INPUT,
        SLOPE_INPUT,
        H0_INPUT,
        H1_INPUT,
        H2_INPUT,
        H3_INPUT,
        H4_INPUT,
        H5_INPUT,
        H6_INPUT,
        H7_INPUT,
        H8_INPUT,
        H9_INPUT,
        H10_INPUT,      // up to here V1.0
        RISE_INPUT,
        FALL_INPUT,
        EVEN_INPUT,
        ODD_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MIX_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        GAIN_GREEN_LIGHT,
        GAIN_RED_LIGHT,
        NUM_LIGHTS
    };
    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<CHBDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void onSampleRateChange()
    {
        knobToFilterL = makeLPFDirectFilterLookup<float>(this->engineGetSampleTime());
    }

    float _freq = 0;

private:
    int cycleCount = 1;
    int clipCount = 0;
    int signalCount = 0;
    const int clipDuration = 4000;
    float finalGain = 0;
    bool isExternalAudio = false;

    static const int polyOrder = 10;

    /**
     * The waveshaper that is the heart of this module.
     * Let's use doubles.
     */
    Poly<double, polyOrder> poly;

    MultiLag<12> lag;

    /*
     * maps freq multiple to "octave".
     * In other words, log base 12.
     */
    float _octave[polyOrder];
    float getOctave(int mult) const;
    void init();

    // round up to 12, so multi-lag is happy
    float _volume[12] = {0};

    /**
     * Internal sine wave oscillator to drive the waveshaper
     */
    SinOscillatorParams<float> sinParams;
    SinOscillatorState<float> sinState;

    // just maps 0..1 to 0..1
    std::shared_ptr<LookupTableParams<float>> audioTaper = {ObjectCache<float>::getAudioTaper()};

    AudioMath::ScaleFun<float> gainCombiner = AudioMath::makeLinearScaler(0.f, 1.f);

    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> db2gain = ObjectCache<float>::getDb2Gain();
    std::shared_ptr <LookupTableParams<float>> knobToFilterL;

    /**
     * Audio taper for the slope.
     */
    AudioMath::ScaleFun<float> slopeScale =
    {AudioMath::makeLinearScaler<float>(-18, 0)};

    /**
     * do one-time calculations when sample rate changes
     */
    void internalUpdate();

    /**
     * Do all the processing to get the input waveform
     * that will be fed to the polynomials
     */
    float getInput();

    void calcVolumes(float *);

    void checkClipping(float sample);

    void updateLagTC();

    /**
     * Does audio taper
     * @param raw = 0..1
     * @return 0..1
     */
    float taper(float raw)
    {
        return LookupTable<float>::lookup(*audioTaper, raw, false);
    }

    AudioMath::ScaleFun<float> lin = AudioMath::makeLinearScaler<float>(0, 1);
};

template <class TBase>
inline void  CHB<TBase>::init()
{
    for (int i = 0; i < polyOrder; ++i) {
        _octave[i] = log2(float(i + 1));
    }
    onSampleRateChange();
    lag.setAttack(.1f);
    lag.setRelease(.0001f);
}

template <class TBase>
inline float  CHB<TBase>::getOctave(int i) const
{
    assert(i >= 0 && i < polyOrder);
    return _octave[i];
}

template <class TBase>
inline void CHB<TBase>::updateLagTC()
{
    const float combinedA = lin(
        TBase::inputs[RISE_INPUT].value,
        TBase::params[PARAM_RISE].value,
        1);

    const float combinedR = lin(
        TBase::inputs[FALL_INPUT].value,
        TBase::params[PARAM_FALL].value,
        1);
    if (combinedA < .1 && combinedR < .1) {
        lag.setEnable(false);
    } else {
        lag.setEnable(true);

        const float lA = LookupTable<float>::lookup(*knobToFilterL, combinedA);
        lag.setAttackL(lA);
        const float lR = LookupTable<float>::lookup(*knobToFilterL, combinedR);
        lag.setReleaseL(lR);
    }
}

template <class TBase>
inline float CHB<TBase>::getInput()
{
    assert(TBase::engineGetSampleTime() > 0);

    // Get the frequency from the inputs.
    float pitch = 1.0f + roundf(TBase::params[PARAM_OCTAVE].value) +
        TBase::params[PARAM_SEMIS].value / 12.0f +
        TBase::params[PARAM_TUNE].value / 12.0f;
    pitch += TBase::inputs[CV_INPUT].value;
    pitch += .25f * TBase::inputs[PITCH_MOD_INPUT].value *
        taper(TBase::params[PARAM_PITCH_MOD_TRIM].value);

    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    _freq = expLookup(pitch);

    if (_freq < .01f) {
        _freq = .01f;
    }

    // Multiply in the Linear FM contribution
    _freq *= 1.0f + TBase::inputs[LINEAR_FM_INPUT].value * taper(TBase::params[PARAM_LINEAR_FM_TRIM].value);
    float time = std::clamp(_freq * TBase::engineGetSampleTime(), -.5f, 0.5f);

    Osc::setFrequency(sinParams, time);

    if (cycleCount == 0) {
        // Get the gain from the envelope generator in
        // eGain = {0 .. 10.0f }
        float eGain = TBase::inputs[ENV_INPUT].active ? TBase::inputs[ENV_INPUT].value : 10.f;
        isExternalAudio = TBase::inputs[AUDIO_INPUT].active;

        const float gainKnobValue = TBase::params[PARAM_EXTGAIN].value;
        const float gainCVValue = TBase::inputs[GAIN_INPUT].value;
        const float gainTrimValue = TBase::params[PARAM_EXTGAIN_TRIM].value;
        const float combinedGain = gainCombiner(gainCVValue, gainKnobValue, gainTrimValue);

        // tapered gain {0 .. 0.5}
        const float taperedGain = .5f * taper(combinedGain);

        // final gain 0..5
        finalGain = taperedGain * eGain;
    }

    float input = finalGain * (isExternalAudio ?
        TBase::inputs[AUDIO_INPUT].value :
        Osc::run(sinState, sinParams));

    checkClipping(input);

    // Now clip or fold to keep in -1...+1
    if (TBase::params[PARAM_FOLD].value > .5) {
        input = AudioMath::fold(input);
    } else {
        input = std::max(input, -1.f);
        input = std::min(input, 1.f);
    }

    return input;
}

/**
 * Desired behavior:
 *      If we clip, led goes red and stays red for clipDuration
 *      if not red, sign present goes green
 *      nothing - turn off
 */
template <class TBase>
inline void CHB<TBase>::checkClipping(float input)
{
    if (input > 1) {
        // if clipping, go red
        clipCount = clipDuration;
        TBase::lights[GAIN_RED_LIGHT].value = 10;
        TBase::lights[GAIN_GREEN_LIGHT].value = 0;
    } else if (clipCount) {
        // If red,run down the clock
        clipCount--;
        if (clipCount <= 0) {
            TBase::lights[GAIN_RED_LIGHT].value = 0;
            TBase::lights[GAIN_GREEN_LIGHT].value = 0;
        }
    } else if (input > .3f) {
        // if signal present
        signalCount = clipDuration;
        TBase::lights[GAIN_GREEN_LIGHT].value = 10;
    } else if (signalCount) {
        signalCount--;
        if (signalCount <= 0) {
            TBase::lights[GAIN_GREEN_LIGHT].value = 0;
        }
    }
}

template <class TBase>
inline void CHB<TBase>::calcVolumes(float * volumes)
{
    // first get the harmonics knobs, and scale them
    for (int i = 0; i < numHarmonics; ++i) {
        float val = taper(TBase::params[i + PARAM_H0].value);       // apply taper to the knobs

        // If input connected, scale and multiply with knob value
        if (TBase::inputs[i + H0_INPUT].active) {
            const float inputCV = TBase::inputs[i + H0_INPUT].value * .1f;
            val *= std::max(inputCV, 0.f);
        }

        volumes[i] = val;
    }

    // Second: apply the even and odd knobs
    {
        const float evenCombined = gainCombiner(
            TBase::inputs[EVEN_INPUT].value,
            TBase::params[PARAM_MAG_EVEN].value,
            TBase::params[PARAM_EVEN_TRIM].value);

        const float oddCombined = gainCombiner(
            TBase::inputs[ODD_INPUT].value,
            TBase::params[PARAM_MAG_ODD].value,
            TBase::params[PARAM_ODD_TRIM].value);

        const float even = taper(evenCombined);
        const float odd = taper(oddCombined);
        for (int i = 1; i < polyOrder; ++i) {
            const float mul = (i & 1) ? even : odd;     // 0 = fundamental, 1=even, 2=odd....
            volumes[i] *= mul;
        }
    }

    // Third: slope
    {
        const float slope = slopeScale(
            TBase::inputs[SLOPE_INPUT].value,
            TBase::params[PARAM_SLOPE].value,
            TBase::params[PARAM_SLOPE_TRIM].value);

        for (int i = 0; i < polyOrder; ++i) {
            float slopeAttenDb = slope * getOctave(i);
            float slopeAtten = LookupTable<float>::lookup(*db2gain, slopeAttenDb);
            volumes[i] *= slopeAtten;
        }
    }
}

template <class TBase>
inline void CHB<TBase>::step()
{
    if (--cycleCount < 0) {
        cycleCount = 3;
    }

    // do all the processing to get the carrier signal
    // Does the pitch every cycle, vol every 4
    const float input = getInput();

    if (cycleCount == 0) {
        updateLagTC();              // TODO: could do at reduced rate
        calcVolumes(_volume);       // now _volume has all 10 harmonic volumes
        lag.step(_volume);          // TODO: we could run lag at full rate.

        for (int i = 0; i < polyOrder; ++i) {
            //poly.setGain(i, _volume[i]);
            poly.setGain(i, lag.get(i));
        }
    }


    float output = poly.run(input, std::min(finalGain, 1.f));
    TBase::outputs[MIX_OUTPUT].value = 5.0f * output;
}


template <class TBase>
int CHBDescription<TBase>::getNumParams()
{
    return CHB<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config CHBDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    const float defaultGainParam = .63108f;

    switch (i) {
        case CHB<TBase>::PARAM_TUNE:
            ret = {-1.0f, 1.0f, 0, "Fine Tune"};
            break;
        case CHB<TBase>::PARAM_OCTAVE:
            ret = {-5.0f, 4.0f, 0.f, "Octave"};
            break;
        case CHB<TBase>::PARAM_EXTGAIN:
            ret = {-5.0f, 5.0f, defaultGainParam, "External Gain"};
            break;
        case CHB<TBase>::PARAM_PITCH_MOD_TRIM:
            ret = {0, 1.0f, 0.0f, "Pitch mod trim"};
            break;
        case CHB<TBase>::PARAM_LINEAR_FM_TRIM:
            ret = {0, 1.0f, 0.0f, "Linear FM trim"};
            break;
        case CHB<TBase>::PARAM_EXTGAIN_TRIM:
            ret = {-1, 1, 0, "External gain trim"};
            break;
        case CHB<TBase>::PARAM_FOLD:
            ret = {0.0f, 1.0f, 0.0f, "Fold/Clip"};
            break;
        case CHB<TBase>::PARAM_SLOPE:
            ret = {-5, 5, 5, "Harmonic slope"};
            break;
        case CHB<TBase>::PARAM_MAG_EVEN:
            ret = {-5, 5, 5, "Even harmonic volume"};
            break;
        case CHB<TBase>::PARAM_MAG_ODD:
            ret = {-5, 5, 5, "Odd harmonic volume"};
            break;
        case CHB<TBase>::PARAM_H0:
            ret = {0.0f, 1.0f, 1, "Fundamental level"};
            break;
        case CHB<TBase>::PARAM_H1:
            ret = {0.0f, 1.0f, 0, "Harmonic 1 level"};
            break;
        case CHB<TBase>::PARAM_H2:
            ret = {0.0f, 1.0f, 0, "Harmonic 2 level"};
            break;
        case CHB<TBase>::PARAM_H3:
            ret = {0.0f, 1.0f, 0, "Harmonic 3 level"};
            break;
        case CHB<TBase>::PARAM_H4:
            ret = {0.0f, 1.0f, 0, "Harmonic 4 level"};
            break;
        case CHB<TBase>::PARAM_H5:
            ret = {0.0f, 1.0f, 0, "Harmonic 5 level"};
            break;
        case CHB<TBase>::PARAM_H6:
            ret = {0.0f, 1.0f, 0, "Harmonic 6 level"};
            break;
        case CHB<TBase>::PARAM_H7:
            ret = {0.0f, 1.0f, 0, "Harmonic 7 level"};
            break;
        case CHB<TBase>::PARAM_H8:
            ret = {0.0f, 1.0f, 0, "Harmonic 8 level"};
            break;
        case CHB<TBase>::PARAM_H9:       // up to here is Ver 1.0
            ret = {0.0f, 1.0f, 0, "Harmonic 9 level"};
            break;
        case CHB<TBase>::PARAM_EXPAND:
            ret = {0, 1, 0, "unused"};
            break;
        case CHB<TBase>::PARAM_RISE:
            ret = {-5.f, 5.f, 0.f, "Rise time (harmonic volume CV)"};
            break;
        case CHB<TBase>::PARAM_FALL:
            ret = {-5.f, 5.f, 0.f, "Fall time (harmonic volume CV)"};
            break;
        case CHB<TBase>::PARAM_EVEN_TRIM:
            ret = {-1.0f, 1.0f, 0, "Even Harmonic CV trim"};
            break;
        case CHB<TBase>::PARAM_ODD_TRIM:
            ret = {-1.0f, 1.0f, 0, "Odd Harmonic CV trim"};
            break;
        case CHB<TBase>::PARAM_SLOPE_TRIM:
            ret = {-1.0f, 1.0f, 0, "Harmonic slope CV trim"};
            break;
        case CHB<TBase>::PARAM_SEMIS:
            ret = {-11.0f, 11.0f, 0.f, "Semitone offset"};
            break;
        default:
            assert(false);
    }
    return ret;
 }

