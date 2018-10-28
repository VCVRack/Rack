#pragma once

#include "StateVariableFilter.h"

// Unfinished single stage eq
class GraphicEq
{
public:

    GraphicEq(int stages, float bw);

    float run(float);
    void setGain(int stage, float g)
    {
        gain[stage] = g;
       // printf("just set gain[%d] to %f\n", stage, g);
    }
private:
    StateVariableFilterParams<float> params[6];
    StateVariableFilterState<float> states[6];
    float gain[6];
    const int _stages;

};

// todo: refactor
inline GraphicEq::GraphicEq(int stages, float bw) : _stages(stages)
{
    assert(stages < 6);
    // .5, 1 stage is 78..128. 2stage 164 273 / 
    // .8  67..148 /  63..314  / 72..456
    const float baseFreq = 100.f / 44100.f;
    float freq = baseFreq;
    for (int i = 0; i < stages; ++i) {
        params[i].setMode(StateVariableFilterParams<float>::Mode::BandPass);
        params[i].setFreq(freq);
        params[i].setNormalizedBandwidth(bw);
        freq *= 2.f;
        gain[i] = 1;

    }
}

inline float GraphicEq::run(float input)
{
   //  printf("run filter with ");
    float out = 0;
    for (int i = 0; i < _stages; ++i) {
     //  printf("%f ", gain[i]);
        out += StateVariableFilter<float>::run(input, states[i], params[i]) * gain[i];
    }
   // printf("\n");
    return out;
}

/**
 * Two bandpass filters in series.
 */
class TwoStageBandpass
{
public:
    TwoStageBandpass();
    float run(float);
    void setFreq(float);
private:
    StateVariableFilterParams<float> params[2];
    StateVariableFilterState<float> state[2];
};

inline TwoStageBandpass::TwoStageBandpass()
{
    for (int i = 0; i <= 1; ++i) {
        params[i].setMode(StateVariableFilterParams<float>::Mode::BandPass);
        params[i].setFreq(.1f);
        params[i].setNormalizedBandwidth(1);
    }
}

inline void TwoStageBandpass::setFreq(float freq)
{
    for (int i = 0; i <= 1; ++i) {
        params[i].setFreq(freq);
    }
}

inline float TwoStageBandpass::run(float input)
{
    auto y = StateVariableFilter<float>::run(input, state[0], params[0]);
    auto z = StateVariableFilter<float>::run(y, state[1], params[1]);
    return z;
}

/**
 * Octave EQ using dual bandpass sections
 * Currently hard-wired to 100 Hz.
 */
template <int NumStages>
class GraphicEq2
{
public:
    GraphicEq2()
    {
        float freq = 100.0f / 44100.0f;
        for (int i = 0; i < NumStages; ++i) {
            filters[i].setFreq(freq);
            freq *= 2.0f;
        }
    }
    float run(float);
    void setGain(int stage, float g)
    {
        assert(stage < NumStages);
        gain[stage] = g;

    }
    int getNumStages()
    {
        return NumStages;
    }
private:
    TwoStageBandpass filters[NumStages];
    float gain[NumStages];
};

template <int NumStages>
inline float GraphicEq2<NumStages>::run(float input)
{
    float out = 0;
    for (int i = 0; i < NumStages; ++i) {
        out += filters[i].run(input) * gain[i];
    }
;
    return out;
}