#pragma once

#include "LookupTable.h"
#include "SinOscillator.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "HilbertFilterDesigner.h"

/**
 * Complete Frequency Shifter composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the Booty Shifter module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */
template <class TBase>
class FrequencyShifter : public TBase
{
public:
    FrequencyShifter(struct Module * module) : TBase(module)
    {
    }
    FrequencyShifter() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
        HilbertFilterDesigner<T>::design(rate, hilbertFilterParamsSin, hilbertFilterParamsCos);
    }

    // must be called after setSampleRate
    void init()
    {
        SinOscillator<T, true>::setFrequency(oscParams, T(.01));
        exponential2 = ObjectCache<T>::getExp2();   // Get a shared copy of the 2**x lookup.
                                                    // This will enable exp mode to track at
                                                    // 1V/ octave.
    }

    // Define all the enums here. This will let the tests and the widget access them.
    enum ParamIds
    {
        PITCH_PARAM,      // the big pitch knob
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        SIN_OUTPUT,
        COS_OUTPUT,
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

    typedef float T;        // use floats for all signals
    T freqRange = 5;        // the freq range switch
private:
    SinOscillatorParams<T> oscParams;
    SinOscillatorState<T> oscState;
    BiquadParams<T, 3> hilbertFilterParamsSin;
    BiquadParams<T, 3> hilbertFilterParamsCos;
    BiquadState<T, 3> hilbertFilterStateSin;
    BiquadState<T, 3> hilbertFilterStateCos;

    std::shared_ptr<LookupTableParams<T>> exponential2;

    float reciprocalSampleRate;
};

template <class TBase>
inline void FrequencyShifter<TBase>::step()
{
    assert(exponential2->isValid());

    // Add the knob and the CV value.
    T freqHz;
    T cvTotal = TBase::params[PITCH_PARAM].value + TBase::inputs[CV_INPUT].value;
    if (cvTotal > 5) {
        cvTotal = 5;
    }
    if (cvTotal < -5) {
        cvTotal = -5;
    }
    if (freqRange > .2) {
        cvTotal *= freqRange;
        cvTotal *= T(1. / 5.);
        freqHz = cvTotal;
    } else {
        cvTotal += 7;           // shift up to GE 2 (min value for out 1v/oct lookup)
        freqHz = LookupTable<T>::lookup(*exponential2, cvTotal);
        freqHz /= 2;            // down to 2..2k range that we want.
    }

    SinOscillator<float, true>::setFrequency(oscParams, freqHz * reciprocalSampleRate);

    // Generate the quadrature sin oscillators.
    T x, y;
    SinOscillator<T, true>::runQuadrature(x, y, oscState, oscParams);

    // Filter the input through th quadrature filter
    const T input = TBase::inputs[AUDIO_INPUT].value;
    const T hilbertSin = BiquadFilter<T>::run(input, hilbertFilterStateSin, hilbertFilterParamsSin);
    const T hilbertCos = BiquadFilter<T>::run(input, hilbertFilterStateCos, hilbertFilterParamsCos);

    // Cross modulate the two sections.
    x *= hilbertSin;
    y *= hilbertCos;

    // And combine for final SSB output.
    TBase::outputs[SIN_OUTPUT].value = x + y;
    TBase::outputs[COS_OUTPUT].value = x - y;
}
