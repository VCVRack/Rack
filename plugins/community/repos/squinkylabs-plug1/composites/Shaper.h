#pragma once


#include "AsymWaveShaper.h"
#include "ButterworthFilterDesigner.h"
#include "IComposite.h"
#include "IIRUpsampler.h"
#include "IIRDecimator.h"
#include "LookupTable.h"
#include "ObjectCache.h"


template <class TBase>
class ShaperDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
Version 1, cpu usage:
    full wave: 95
    crush: 281
    asy:149
    fold: 102
    fold2: 154

    X4 on input scanning:
    full wave: 85
    crush: 278
    asy:163
    fold: 89
    fold2: 154

    inline:
    fw: 75
    crush: 87
    asy: 112
    fold: 77
    fold2: 136


 */
template <class TBase>
class Shaper : public TBase
{
public:
    Shaper(struct Module * module) : TBase(module)
    {
        init();
    }
    Shaper() : TBase()
    {
        init();
    }

    void onSampleRateChange();

    enum class Shapes
    {
        AsymSpline,
        Clip,
        EmitterCoupled,
        FullWave,
        HalfWave,
        Fold,
        Fold2,
        Crush,
        Invalid
    };

    static const char* getString(Shapes);

    enum ParamIds
    {
        PARAM_SHAPE,
        PARAM_GAIN,
        PARAM_GAIN_TRIM,
        PARAM_OFFSET,
        PARAM_OFFSET_TRIM,
        PARAM_OVERSAMPLE,
        PARAM_ACDC,
        NUM_PARAMS
    };

    enum InputIds
    {
        INPUT_AUDIO,
        INPUT_GAIN,
        INPUT_OFFSET,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUTPUT_AUDIO,
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
        return std::make_shared<ShaperDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    float _gain = 0;
    float _offset = 0;
    float _gainInput = 0;

private:
    std::shared_ptr<LookupTableParams<float>> audioTaper = {ObjectCache<float>::getAudioTaper()};
    std::shared_ptr<LookupTableParams<float>> sinLookup = {ObjectCache<float>::getSinLookup()};
    AudioMath::ScaleFun<float> scaleGain = AudioMath::makeLinearScaler<float>(0, 1);
    AudioMath::ScaleFun<float> scaleOffset = AudioMath::makeLinearScaler<float>(-5, 5);

    const static int maxOversample = 16;
    int curOversample = 16;
    void init();
    IIRUpsampler up;
    IIRDecimator dec;
    std::shared_ptr<LookupTableParams<float>> tanhLookup;
    AsymWaveShaper asymShaper;
    int cycleCount = 0;
    Shapes shape = Shapes::Clip;
    int asymCurveindex = 0;

    /**
     * 4 pole butterworth HP
     */
    using Thpf = double;
    BiquadParams<Thpf, 2> dcBlockParams;
    BiquadState<Thpf, 2> dcBlockState;


    void processCV();
    void setOversample();
    void processBuffer(float *) const;
};

template <class TBase>
const char* Shaper<TBase>::getString(Shapes shape)
{
    const char* ret = "";
    switch (shape) {
        case Shapes::Clip:
            ret = "Clip";
            break;
        case Shapes::EmitterCoupled:
            ret = "Emitter Coupled";
            break;
        case Shapes::FullWave:
            ret = "Full Wave";
            break;
        case Shapes::HalfWave:
            ret = "Half Wave";
            break;
        case Shapes::Fold:
            ret = "Folder";
            break;
        case Shapes::Fold2:
            ret = "Folder II";
            break;
        case Shapes::AsymSpline:
            ret = "Smooth";
            break;
        case Shapes::Crush:
            ret = "Crush";
            break;
        default:
            assert(false);
            ret = "error";
    }
    return ret;
}


template <class TBase>
void  Shaper<TBase>::init()
{
    onSampleRateChange();
    setOversample();
    tanhLookup = ObjectCache<float>::getTanh5();
}

template <class TBase>
void  Shaper<TBase>::setOversample()
{
    //   float fc = .25 / float(oversample);
    up.setup(curOversample);
    dec.setup(curOversample);
}

template <class TBase>
void  Shaper<TBase>::onSampleRateChange()
{
    const float cutoffHz = 20.f;
    float fcNormalized = cutoffHz * this->engineGetSampleTime();
    assert((fcNormalized > 0) && (fcNormalized < .1));
    ButterworthFilterDesigner<Thpf>::designFourPoleHighpass(dcBlockParams, fcNormalized);
}

template <class TBase>
void Shaper<TBase>::processCV()
{
    int oversampleCode = (int) std::round(TBase::params[PARAM_OVERSAMPLE].value);
    switch (oversampleCode) {
        case 0:
            curOversample = 16;
            setOversample();
            break;
        case 1:
            curOversample = 4;
            setOversample();
            break;
        case 2:
            curOversample = 1;
            break;
        default:
            assert(false);
    }

    // 0..1
    _gainInput = scaleGain(
        TBase::inputs[INPUT_GAIN].value,
        TBase::params[PARAM_GAIN].value,
        TBase::params[PARAM_GAIN_TRIM].value);

    _gain = 5 * LookupTable<float>::lookup(*audioTaper, _gainInput, false);


    // -5 .. 5
    const float offsetInput = scaleOffset(
        TBase::inputs[INPUT_OFFSET].value,
        TBase::params[PARAM_OFFSET].value,
        TBase::params[PARAM_OFFSET_TRIM].value);

    _offset = offsetInput;

    const int iShape = (int) std::round(TBase::params[PARAM_SHAPE].value);
    shape = Shapes(iShape);

    const float sym = .1f * (5 - _offset);
    asymCurveindex = (int) round(sym * 15.1);           // This math belongs in the shaper
}

template <class TBase>
void  Shaper<TBase>::step()
{
    if (--cycleCount < 0) {
        cycleCount = 7;
        processCV();
    }

    float buffer[maxOversample];
    float input = TBase::inputs[INPUT_AUDIO].value;
    // const float rawInput = input;

    // TODO: maybe add offset after gain?
    if (shape != Shapes::AsymSpline) {
        input += _offset;
    }
    if (shape != Shapes::Crush) {
        input *= _gain;
    }

    if (curOversample != 1) {
        up.process(buffer, input);
    } else {
        buffer[0] = input;
    }

    processBuffer(buffer);
    float output;
    if (curOversample != 1) {
        output = dec.process(buffer);
    } else {
        output = buffer[0];
    }

    if (TBase::params[PARAM_ACDC].value < .5) {
        output = float(BiquadFilter<Thpf>::run(output, dcBlockState, dcBlockParams));
    }
    TBase::outputs[OUTPUT_AUDIO].value = output;
}


template <class TBase>
void  Shaper<TBase>::processBuffer(float* buffer) const
{
    switch (shape) {
        case Shapes::FullWave:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x = std::abs(x);
                x *= 1.94f;
                x = std::min(x, 10.f);
                buffer[i] = x;
            }
            break;
        case  Shapes::AsymSpline:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x *= .15f;
                x = asymShaper.lookup(x, asymCurveindex);
                x *= 6.1f;
                buffer[i] = x;
            }
            break;
        case Shapes::Clip:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x *= 3;
                x = std::min(3.f, x);
                x = std::max(-3.f, x);
                x *= 1.2f;
                buffer[i] = x;
            }
            break;
        case Shapes::EmitterCoupled:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x *= .25;
                x = LookupTable<float>::lookup(*tanhLookup.get(), x, true);
                x *= 5.4f;
                buffer[i] = x;
            }
            break;

        case Shapes::HalfWave:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x = std::max(0.f, x);
                x *= 1.4f * 1.26f;
                x = std::min(x, 10.f);
                buffer[i] = x;
            }
            break;
        case Shapes::Fold:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x = AudioMath::fold(x);
                x *= 5.6f;
                buffer[i] = x;
            }
            break;
        case Shapes::Fold2:
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];
                x = .3f * AudioMath::fold(x);
                if (x > 0) {
                    x = LookupTable<float>::lookup(*sinLookup, 1.3f * x, false);
                } else {
                    x = -LookupTable<float>::lookup(*sinLookup, -x, false);
                }
                if (x > 0) x = std::sqrt(x);
                x *= 4.4f;
                buffer[i] = x;
            }
            break;

        case Shapes::Crush:
        {
            float invGain = 1 + (1 - _gainInput) * 100; //0..10
            invGain *= .01f;
            invGain = std::max(invGain, .09f);
            assert(invGain >= .09);
            for (int i = 0; i < curOversample; ++i) {
                float x = buffer[i];            // for crush, no gain has been applied

                x *= invGain;
                x = std::round(x + .5f) - .5f;
                x /= invGain;
                buffer[i] = x;
            }
        }
        break;

        default:
            assert(false);
    }

}

template <class TBase>
int ShaperDescription<TBase>::getNumParams()
{
    return Shaper<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config ShaperDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");

    switch (i) {
        case Shaper<TBase>::PARAM_SHAPE:
            ret = {0,
                float(Shaper<TBase>::Shapes::Invalid) - 1,
                0,
                "Shape"};
            break;

        case Shaper<TBase>::PARAM_GAIN:
            ret = {-5, 5, 0, "Gain"};
            break;
        case Shaper<TBase>::PARAM_GAIN_TRIM:
            ret = {-1, 1, 0, "Gain trim"};
            break;
        case Shaper<TBase>::PARAM_OFFSET:
            ret = {-5, 5, 0, "Offset/Bias"};
            break;
        case Shaper<TBase>::PARAM_OFFSET_TRIM:
            ret = {-1, 1, 0, "Offset trim"};
            break;
        case Shaper<TBase>::PARAM_OVERSAMPLE:
            ret = {0.0f, 2.f, 0, "Oversample"};
            break;
        case Shaper<TBase>::PARAM_ACDC:
            ret = {0.0f, 1.f, 0, "AC/DC"};
            break;
        default:
            assert(false);
    }

    return ret;
}