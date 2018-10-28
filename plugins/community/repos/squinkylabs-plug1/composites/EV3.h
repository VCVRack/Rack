#pragma once

#include "MinBLEPVCO.h"
#include "ObjectCache.h"

#include "dsp/functions.hpp"        // rack math

/**
 *
 */
template <class TBase>
class EV3 : public TBase
{
public:
    friend class TestMB;
    EV3(struct Module * module) : TBase(module)
    {
        init();
    }

    EV3() : TBase()
    {
        init();
    }

    enum class Waves
    {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        NONE,
        END     // just a marker
    };

    enum ParamIds
    {
        MIX1_PARAM,
        MIX2_PARAM,
        MIX3_PARAM,
        OCTAVE1_PARAM,
        SEMI1_PARAM,
        FINE1_PARAM,
        FM1_PARAM,
        SYNC1_PARAM,
        WAVE1_PARAM,
        PW1_PARAM,
        PWM1_PARAM,

        OCTAVE2_PARAM,
        SEMI2_PARAM,
        FINE2_PARAM,
        FM2_PARAM,
        SYNC2_PARAM,
        WAVE2_PARAM,
        PW2_PARAM,
        PWM2_PARAM,

        OCTAVE3_PARAM,
        SEMI3_PARAM,
        FINE3_PARAM,
        FM3_PARAM,
        SYNC3_PARAM,
        WAVE3_PARAM,
        PW3_PARAM,
        PWM3_PARAM,

        NUM_PARAMS
    };

    enum InputIds
    {
        CV1_INPUT,
        CV2_INPUT,
        CV3_INPUT,
        FM1_INPUT,
        FM2_INPUT,
        FM3_INPUT,
        PWM1_INPUT,
        PWM2_INPUT,
        PWM3_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MIX_OUTPUT,
        VCO1_OUTPUT,
        VCO2_OUTPUT,
        VCO3_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    void step() override;

private:
    void setSync();
    void processPitchInputs();
    void processPitchInputs(int osc);
    void processWaveforms();
    void stepVCOs();
    void init();
    void processPWInputs();
    void processPWInput(int osc);
    float getInput(int osc, InputIds in0, InputIds in1, InputIds in2);

    MinBLEPVCO vcos[3];
    float _freq[3];
    float _out[3];
    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();
};

template <class TBase>
inline void EV3<TBase>::init()
{
    for (int i = 0; i < 3; ++i) {
        vcos[i].setWaveform(MinBLEPVCO::Waveform::Saw);
    }

    vcos[0].setSyncCallback([this](float f) {

        if (TBase::params[SYNC2_PARAM].value > .5) {
            vcos[1].onMasterSync(f);
        }
        if (TBase::params[SYNC3_PARAM].value > .5) {
            vcos[2].onMasterSync(f);
        }
        });
}

template <class TBase>
inline void EV3<TBase>::setSync()
{
    vcos[0].setSyncEnabled(false);
    vcos[1].setSyncEnabled(TBase::params[SYNC2_PARAM].value > .5);
    vcos[2].setSyncEnabled(TBase::params[SYNC3_PARAM].value > .5);
}

template <class TBase>
inline void EV3<TBase>::processWaveforms()
{
    vcos[0].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE1_PARAM].value);
    vcos[1].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE2_PARAM].value);
    vcos[2].setWaveform((MinBLEPVCO::Waveform)(int)TBase::params[WAVE3_PARAM].value);
}

template <class TBase>
float EV3<TBase>::getInput(int osc, InputIds in1, InputIds in2, InputIds in3)
{
    const bool in2Connected = TBase::inputs[in2].active;
    const bool in3Connected = TBase::inputs[in3].active;
    InputIds id = in1;
    if ((osc == 1) && in2Connected) {
        id = in2;
    }
    if (osc == 2) {
        if (in3Connected) id = in3;
        else if (in2Connected)  id = in2;
    }
    return TBase::inputs[id].value;
}

template <class TBase>
void EV3<TBase>::processPWInput(int osc)
{
    const float pwmInput = getInput(osc, PWM1_INPUT, PWM2_INPUT, PWM3_INPUT) / 5.f;

    const int delta = osc * (OCTAVE2_PARAM - OCTAVE1_PARAM);
    const float pwmTrim = TBase::params[PWM1_PARAM + delta].value;
    const float pwInit = TBase::params[PW1_PARAM + delta].value;

    float pw = pwInit + pwmInput * pwmTrim;
    const float minPw = 0.05f;
    pw = rack::rescale(std::clamp(pw, -1.0f, 1.0f), -1.0f, 1.0f, minPw, 1.0f - minPw);
    vcos[osc].setPulseWidth(pw);
}

template <class TBase>
inline void EV3<TBase>::processPWInputs()
{
    processPWInput(0);
    processPWInput(1);
    processPWInput(2);
}

template <class TBase>
inline void EV3<TBase>::step()
{
    setSync();
    processPitchInputs();
    processWaveforms();
    processPWInputs();
    stepVCOs();
    float mix = 0;

    for (int i = 0; i < 3; ++i) {

        const float knob = TBase::params[MIX1_PARAM + i].value;
        const float gain = LookupTable<float>::lookup(*audioTaper, knob, false);
        const float rawWaveform = vcos[i].getOutput();
        const float scaledWaveform = rawWaveform * gain;
        mix += scaledWaveform;
        _out[i] = scaledWaveform;
        TBase::outputs[VCO1_OUTPUT + i].value = rawWaveform;
    }
    TBase::outputs[MIX_OUTPUT].value = mix;
}

template <class TBase>
inline void EV3<TBase>::stepVCOs()
{
    for (int i = 0; i < 3; ++i) {
        vcos[i].step();
    }
}

template <class TBase>
inline void EV3<TBase>::processPitchInputs()
{
    float lastFM = 0;
    for (int osc = 0; osc < 3; ++osc) {
        assert(osc >= 0 && osc <= 2);
        const int delta = osc * (OCTAVE2_PARAM - OCTAVE1_PARAM);

        const float cv = getInput(osc, CV1_INPUT, CV2_INPUT, CV3_INPUT);
        const float finePitch = TBase::params[FINE1_PARAM + delta].value / 12.0f;
        const float semiPitch = TBase::params[SEMI1_PARAM + delta].value / 12.0f;
       // const float fm = getInput(osc, FM1_INPUT, FM2_INPUT, FM3_INPUT);

        float pitch = 1.0f + roundf(TBase::params[OCTAVE1_PARAM + delta].value) +
            semiPitch +
            finePitch;
        pitch += cv;

        float fmCombined = 0;       // The final, scaled, value (post knob
        if (TBase::inputs[FM1_INPUT + osc].active) {
            const float fm = TBase::inputs[FM1_INPUT + osc].value;
           // const float fmKnob = TBase::params[FM1_PARAM + delta].value;
            //const float fmDepth = LookupTable<float>::lookup(*audioTaper, fmKnob, false);
            const float fmDepth = rack::quadraticBipolar(TBase::params[FM1_PARAM + delta].value);

            fmCombined = (fmDepth * fm);
#if 0
            static float biggest = 0;
            if (fmCombined > biggest) {
                printf("CV =%f knob = %f depth=%f combined=%f\n", fm, fmKnob, fmDepth, fmCombined);
                fflush(stdout);
                biggest = fmCombined;
            }
#endif

           // pitch += (fmDepth * fm * 12);
            } else {
            fmCombined = lastFM;
        }
        pitch += fmCombined;
        lastFM = fmCombined;


        const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
        pitch += q;
        const float freq = expLookup(pitch);
        _freq[osc] = freq;
        vcos[osc].setNormalizedFreq(TBase::engineGetSampleTime() * freq,
            TBase::engineGetSampleTime());
    }
}



