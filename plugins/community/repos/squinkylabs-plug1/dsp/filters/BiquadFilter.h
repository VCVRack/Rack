#pragma once

template<typename T, int N> class BiquadState;
template<typename T, int N> class BiquadParams;

#include "BiquadParams.h"// what is our forward declaration strategy here? put implementation in c++?
#include "DspFilter.h"	 // TODO: get rid of this. we need it now because you can't forward declare Cascade::Stage

/*
 *
 *                  (I call this node x in the implementation)
 *                    |
 *
 * >----- + ----------|--> b0 >-- + -------->
 *        |           |           |
 *        |          Z-1          |
 *        |           |           |
 *        |--< -a1 <--|--> b1 >----
 *        |           |           |
 *        |          Z-1          |
 *        |           |           |
 *        |           |           |
 *        |--< -a2 <--|--> b2 >----
 *
 *
 *
 */
template<typename T>
class BiquadFilter
{
public:
    BiquadFilter() = delete;       // we are only static
    template<int N>
    static T run(T input, BiquadState<T, N>& state, const BiquadParams<T, N>& params);

    /**
     * Translate filter coefficients from Dsp:: conventions to DspParam structure
     */
    template<int N>
    static void fillFromStages(BiquadParams<T, N>& params, Dsp::Cascade::Stage * stages, int numStages);
};

template <typename T>
template<int N>
inline void BiquadFilter<T>::fillFromStages(BiquadParams<T, N>& params, Dsp::Cascade::Stage * stages, int numStages)
{
    assert(numStages == N);

    for (int i = 0; i < N; ++i) {
        const Dsp::Cascade::Stage& stage = stages[i];

        params.B0(i) = (T) stage.b[0];
        params.B1(i) = (T) stage.b[1];
        params.B2(i) = (T) stage.b[2];
        params.A1(i) = (T) stage.a[1];
        params.A2(i) = (T) stage.a[2];
    }
}

template<typename T>
template<int N>
inline T BiquadFilter<T>::run(T input, BiquadState<T, N>& state, const BiquadParams<T, N>& params)
{

    for (int stage = 0; stage < N; ++stage) {
        T x = input + (params.A1(stage) * state.z0(stage) + params.A2(stage) * state.z1(stage));

        input = params.B0(stage) * x +
            params.B1(stage) * state.z0(stage) +
            params.B2(stage) * state.z1(stage);
        state.z1(stage) = state.z0(stage);
        state.z0(stage) = x;
    }
    return input;
}
