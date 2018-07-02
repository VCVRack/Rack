#pragma once

#include "AudioMath.h"
#include "LookupTable.h"
#include "ObjectCache.h"
#include "SawOscillator.h"

template<typename T> class SinOscillatorParams;
template<typename T> class SinOscillatorState;

/**
 * A simple sin oscillator based on lookup table.
 * Advantage:
 *      fast.
 *      frequency may be changed without phase discontinuity.
 *      Optional quadrature output.
 * Disadvantage:
 *      Not the best spectral purity (significant amount of phase jitter)
 */
template<typename T, bool frequencyCanBeNegative>
class SinOscillator
{
public:
    SinOscillator() = delete;       // we are only static
    static void setFrequency(SinOscillatorParams<T>&, T frequency);
    static T run(SinOscillatorState<T>&, const SinOscillatorParams<T>&);
    static void runQuadrature(T& output, T& outputQuadrature, SinOscillatorState<T>&, const SinOscillatorParams<T>&);
};

template<typename T, bool frequencyCanBeNegative>
inline void SinOscillator<T, frequencyCanBeNegative>::setFrequency(SinOscillatorParams<T>& params, T frequency)
{

    std::function<double(double)> f = AudioMath::makeFunc_Sin();

    // TODO: figure out a better initialization strategy
    // and a better strategy for table size
    // with 4096 thd was -130 db. let's use less memory!
  // if (!params.lookupParams.isValid()) {
 //       LookupTable<T>::init(params.lookupParams, 256, 0, 1, f);
  //  }
    assert(params.lookupParams->isValid());

    SawOscillator<T, true>::setFrequency(params.sawParams, frequency);
}

template<typename T, bool frequencyCanBeNegative>
inline T SinOscillator<T, frequencyCanBeNegative>::run(
    SinOscillatorState<T>& state, const SinOscillatorParams<T>& params)
{

    const T temp = SawOscillator<T, frequencyCanBeNegative>::runSaw(state.sawState, params.sawParams);
    const T ret = LookupTable<T>::lookup(*params.lookupParams, temp);
    return ret;
}

template<typename T, bool frequencyCanBeNegative>
inline void SinOscillator<T, frequencyCanBeNegative>::runQuadrature(
    T& output, T& outputQuadrature, SinOscillatorState<T>& state, const SinOscillatorParams<T>& params)
{

    T saw, quadratureSaw;
    SawOscillator<T, frequencyCanBeNegative>::runQuadrature(saw, quadratureSaw, state.sawState, params.sawParams);
    output = LookupTable<T>::lookup(*params.lookupParams, saw);
    outputQuadrature = LookupTable<T>::lookup(*params.lookupParams, quadratureSaw);
};

template<typename T>
class SinOscillatorParams
{
public:
    SawOscillatorParams<T> sawParams;
   // LookupTableParams<T> lookupParams;
    std::shared_ptr<LookupTableParams<T>> lookupParams;
    SinOscillatorParams()
    {
        lookupParams = ObjectCache<T>::getSinLookup();
    }
    SinOscillatorParams(const SinOscillatorParams&) = delete;
};

template<typename T>
class SinOscillatorState
{
public:
    SawOscillatorState<T> sawState;
};
