#pragma once

#include "StateVariableFilter.h"

class StateVariable4PHP
{
public:
    StateVariable4PHP();

    float run(float);
    void setCutoff(float);
private:
    StateVariableFilterParams<float> params1;
    StateVariableFilterParams<float> params2;

    StateVariableFilterState<float> state1;
    StateVariableFilterState<float> state2;

};

inline StateVariable4PHP::StateVariable4PHP()
{
    params1.setMode(StateVariableFilterParams<float>::Mode::HiPass);
    params1.setQ(.54119f);

    params2.setMode(StateVariableFilterParams<float>::Mode::HiPass);
    params2.setQ(1.30656296f);
}

inline float StateVariable4PHP::run(float input)
{
    float output = StateVariableFilter<float>::run(input, state1, params1);
    output = StateVariableFilter<float>::run(output, state2, params2);
    return output;
}

inline void StateVariable4PHP::setCutoff(float fc)
{
    params1.setFreq(fc);
    params2.setFreq(fc);
}