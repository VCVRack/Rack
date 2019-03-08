#pragma once


#include "FunVCO.h"

#include "IComposite.h"

template <class TBase>
class FunDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class FunVCOComposite : public TBase
{
public:
    FunVCOComposite()
    {
        init();
    }
    FunVCOComposite(struct Module * module) : TBase(module)
    {
        init();
    }
    enum ParamIds
    {
        MODE_PARAM,
        SYNC_PARAM,
        FREQ_PARAM,
        FINE_PARAM,
        FM_PARAM,
        PW_PARAM,
        PWM_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH_INPUT,
        FM_INPUT,
        SYNC_INPUT,
        PW_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        SIN_OUTPUT,
        TRI_OUTPUT,
        SAW_OUTPUT,
        SQR_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<FunDescription<TBase>>();
    }

    void step() override;
    void init()
    {
        oscillator.init();
    }

    void setSampleRate(float rate)
    {
        oscillator.sampleTime = 1.f / rate;
    }

private:
#ifdef _ORIGVCO
    VoltageControlledOscillatorOrig<16, 16> oscillator;
#else
    VoltageControlledOscillator<16, 16> oscillator;
#endif
};


template <class TBase>
int FunDescription<TBase>::getNumParams()
{
    return FunVCOComposite<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config FunDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch(i) {
        case FunVCOComposite<TBase>::MODE_PARAM:
            ret = {0.0f, 1.0f, 1.0f, "Analog/digital mode"};
            break;
        case FunVCOComposite<TBase>::SYNC_PARAM:
            ret = {0.0f, 1.0f, 1.0f, "Sync hard/soft"};
            break;
        case FunVCOComposite<TBase>::FREQ_PARAM:
            ret = {-54.0f, 54.0f, 0.0f, "Frequency"};
            break;
        case FunVCOComposite<TBase>::FINE_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Fine frequency"};
            break;
        case FunVCOComposite<TBase>::FM_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Pitch modulation depth"};
            break;
        case FunVCOComposite<TBase>::PW_PARAM:
            ret = {0.0f, 1.0f, 0.5f, "Pulse width"};
            break;
        case FunVCOComposite<TBase>::PWM_PARAM:
            ret = {0.0f, 1.0f, 0.0f, "Pulse width modulation depth"};
            break;
        default:
            assert(false);
    }
    return ret;
}


template <class TBase>
inline void FunVCOComposite<TBase>::step()
{
    oscillator.analog = TBase::params[MODE_PARAM].value > 0.0f;
    oscillator.soft = TBase::params[SYNC_PARAM].value <= 0.0f;

    float pitchFine = 3.0f * sq::quadraticBipolar(TBase::params[FINE_PARAM].value);
    float pitchCv = 12.0f * TBase::inputs[PITCH_INPUT].value;
    if (TBase::inputs[FM_INPUT].active) {
        pitchCv += sq::quadraticBipolar(TBase::params[FM_PARAM].value) * 12.0f * TBase::inputs[FM_INPUT].value;
    }

    oscillator.setPitch(TBase::params[FREQ_PARAM].value, pitchFine + pitchCv);


    oscillator.setPulseWidth(TBase::params[PW_PARAM].value + TBase::params[PWM_PARAM].value * TBase::inputs[PW_INPUT].value / 10.0f);
    oscillator.syncEnabled = TBase::inputs[SYNC_INPUT].active;

#ifndef _ORIGVCO
    oscillator.sawEnabled = TBase::outputs[SAW_OUTPUT].active;
    oscillator.sinEnabled = TBase::outputs[SIN_OUTPUT].active;
    oscillator.sqEnabled = TBase::outputs[SQR_OUTPUT].active;
    oscillator.triEnabled = TBase::outputs[TRI_OUTPUT].active;
#endif

    oscillator.process(TBase::engineGetSampleTime(), TBase::inputs[SYNC_INPUT].value);
    // Set output
    if (TBase::outputs[SIN_OUTPUT].active)
        TBase::outputs[SIN_OUTPUT].value = 5.0f * oscillator.sin();
    if (TBase::outputs[TRI_OUTPUT].active)
        TBase::outputs[TRI_OUTPUT].value = 5.0f * oscillator.tri();
    if (TBase::outputs[SAW_OUTPUT].active)
        TBase::outputs[SAW_OUTPUT].value = 5.0f * oscillator.saw();
    if (TBase::outputs[SQR_OUTPUT].active)
        TBase::outputs[SQR_OUTPUT].value = 5.0f * oscillator.sqr();

}