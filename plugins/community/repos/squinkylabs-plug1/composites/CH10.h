
#pragma once

#include "ObjectCache.h"
#include "Poly.h"
#include "SinOscillator.h"


template <class TBase>
class CH10 : public TBase
{
public:

    CH10(struct Module * module) : TBase(module)
    {
    }
    CH10() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        A0_PARAM,
        A1_PARAM,
        A2_PARAM,
        A3_PARAM,
        A4_PARAM,
        A5_PARAM,
        A6_PARAM,
        A7_PARAM,
        A8_PARAM,
        A9_PARAM,
        B0_PARAM,
        B1_PARAM,
        B2_PARAM,
        B3_PARAM,
        B4_PARAM,
        B5_PARAM,
        B6_PARAM,
        B7_PARAM,
        B8_PARAM,
        B9_PARAM,
        A0B0_PARAM,
        A0B1_PARAM,
        A0B2_PARAM,
        A0B3_PARAM,
        A0B4_PARAM,
        A0B5_PARAM,
        A0B6_PARAM,
        A0B7_PARAM,
        A0B8_PARAM,
        A0B9_PARAM,
        A1B0_PARAM,
        A1B1_PARAM,
        A1B2_PARAM,
        A1B3_PARAM,
        A1B4_PARAM,
        A1B5_PARAM,
        A1B6_PARAM,
        A1B7_PARAM,
        A1B8_PARAM,
        A1B9_PARAM,
        A2B0_PARAM,
        A2B1_PARAM,
        A2B2_PARAM,
        A2B3_PARAM,
        A2B4_PARAM,
        A2B5_PARAM,
        A2B6_PARAM,
        A2B7_PARAM,
        A2B8_PARAM,
        A2B9_PARAM,
        A3B0_PARAM,
        A3B1_PARAM,
        A3B2_PARAM,
        A3B3_PARAM,
        A3B4_PARAM,
        A3B5_PARAM,
        A3B6_PARAM,
        A3B7_PARAM,
        A3B8_PARAM,
        A3B9_PARAM,
        A4B0_PARAM,
        A4B1_PARAM,
        A4B2_PARAM,
        A4B3_PARAM,
        A4B4_PARAM,
        A4B5_PARAM,
        A4B6_PARAM,
        A4B7_PARAM,
        A4B8_PARAM,
        A4B9_PARAM,
        A5B0_PARAM,
        A5B1_PARAM,
        A5B2_PARAM,
        A5B3_PARAM,
        A5B4_PARAM,
        A5B5_PARAM,
        A5B6_PARAM,
        A5B7_PARAM,
        A5B8_PARAM,
        A5B9_PARAM,
        A6B0_PARAM,
        A6B1_PARAM,
        A6B2_PARAM,
        A6B3_PARAM,
        A6B4_PARAM,
        A6B5_PARAM,
        A6B6_PARAM,
        A6B7_PARAM,
        A6B8_PARAM,
        A6B9_PARAM,
        A7B0_PARAM,
        A7B1_PARAM,
        A7B2_PARAM,
        A7B3_PARAM,
        A7B4_PARAM,
        A7B5_PARAM,
        A7B6_PARAM,
        A7B7_PARAM,
        A7B8_PARAM,
        A7B9_PARAM,
        A8B0_PARAM,
        A8B1_PARAM,
        A8B2_PARAM,
        A8B3_PARAM,
        A8B4_PARAM,
        A8B5_PARAM,
        A8B6_PARAM,
        A8B7_PARAM,
        A8B8_PARAM,
        A8B9_PARAM,
        A9B0_PARAM,
        A9B1_PARAM,
        A9B2_PARAM,
        A9B3_PARAM,
        A9B4_PARAM,
        A9B5_PARAM,
        A9B6_PARAM,
        A9B7_PARAM,
        A9B8_PARAM,
        A9B9_PARAM,
        AOCTAVE_PARAM,
        BOCTAVE_PARAM,
        ASEMI_PARAM,
        BSEMI_PARAM,
        ATUNE_PARAM,
        BTUNE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        ACV_INPUT,
        BCV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MIXED_OUTPUT,
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
    using Osc = SinOscillator<float, true>;
    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    static const int polyOrder = 10;
    class VCOState
    {
    public:
        SinOscillatorParams<float> sinParams;
        SinOscillatorState<float> sinState;
        Poly<double, polyOrder> poly;
    };

    VCOState vcoState[2];

    void updatePitch();
    void updateVCOs(int which);
    void updateAudio();
    void runVCOs(int which);
    void doOutput();
};


template <class TBase>
inline void CH10<TBase>::init()
{
}


template <class TBase>
inline void CH10<TBase>::step()
{
    updatePitch();
    updateAudio();
}

template <class TBase>
inline void CH10<TBase>::updatePitch()
{
    updateVCOs(0);
    updateVCOs(1);
}


template <class TBase>
inline void CH10<TBase>::updateVCOs(int which)
{
    const float cv = TBase::inputs[ACV_INPUT + which].value;
   // const float finePitch = TBase::params[FINE1_PARAM + delta].value / 12.0f;
    const float finePitch = 0;
    const float semiPitch = TBase::params[ASEMI_PARAM + which].value / 12.0f;
    float pitch = 1.0f + roundf(TBase::params[AOCTAVE_PARAM + which].value) +
        semiPitch +
        finePitch;
    pitch += cv;
    //
    const float q = float(log2(261.626));       // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = expLookup(pitch);


    float time = freq * TBase::engineGetSampleTime();

    Osc::setFrequency(vcoState[which].sinParams, time);
       // vcos[osc].setNormalizedFreq(TBase::engineGetSampleTime() * freq,
        //    TBase::engineGetSampleTime());
}

template <class TBase>
inline void CH10<TBase>::runVCOs(int which)
{
    CH10<TBase>::VCOState& vco = vcoState[which];
    float sinOsc = Osc::run(vco.sinState, vco.sinParams);

    vco.poly.run(sinOsc, 1);
}

template <class TBase>
inline void CH10<TBase>::updateAudio()
{
    runVCOs(0);
    runVCOs(1);
    doOutput();
}

template <class TBase>
inline void CH10<TBase>::doOutput()
{
    float sum = 0;
    int num = 0;
    for (int i = 0; i < polyOrder; ++i) {
        if (TBase::params[A0_PARAM + i].value > .5f) {
            sum += vcoState[0].poly.getOutput(i);
            ++num;
        }
    }

    for (int i = 0; i < polyOrder; ++i) {
        if (TBase::params[B0_PARAM + i].value > .5f) {
            sum += vcoState[1].poly.getOutput(i);
            ++num;
        }
    }

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            //float x = gridCol1 + gridSize * (col+1);
            //float y = gridRow1 - row * gridSize;
            int id = CH10<Widget>::A0B0_PARAM + col + row * 10;
            if (TBase::params[id].value > .5f) {
                const float ring = vcoState[0].poly.getOutput(col) *
                    vcoState[1].poly.getOutput(row);
                sum += ring;
                ++num;
            }
        }
    }

    TBase::outputs[MIXED_OUTPUT].value = sum / num;
}
