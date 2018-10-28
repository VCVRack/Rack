#pragma once

#include <assert.h>
#include "AudioMath.h"

template <typename T>
class LowpassFilterState
{
public:
    T z=0;
};

template <typename T>
class LowpassFilterParams
{
public:
    T k=0;
    T l=0;
};

template <typename T>
class LowpassFilter
{
public:
    /**
     * fs is normalize frequency
     */
    static void setCutoff(LowpassFilterParams<T>& params, T fs);

    static T run(T input, LowpassFilterState<T>& state, const LowpassFilterParams<T>& params);
};

template <typename T>
inline  void LowpassFilter<T>::setCutoff(LowpassFilterParams<T>& params, T fs)
{
    assert(fs > 00 && fs < .5);
    params.k = T(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    params.l = T(1.0 - params.k);
}


/*
void go_dbl(double x)
{
_z = _z * _l + _k * x;
}
*/
template <typename T>
inline T LowpassFilter<T>::run(T input, LowpassFilterState<T>& state, const LowpassFilterParams<T>& params)
{
    state.z = state.z * params.l + params.k * input;
    return state.z;
}