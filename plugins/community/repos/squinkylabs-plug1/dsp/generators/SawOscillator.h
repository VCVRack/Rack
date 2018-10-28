#pragma once

/**
 * SawOscillator
 *
 * Good features:
 *      frequency may be adjusted while running without glitches
 *      can generate a quadrature output
 *      can be configured to do positive and negative frequency
 */
template<typename T> class SawOscillatorParams;
template<typename T> class SawOscillatorState;

template<typename T, bool frequencyCanBeNegative>
class SawOscillator
{
public:
    SawOscillator() = delete;       // we are only static
    static void setFrequency(SawOscillatorParams<T>& params, T freq);   // TODO: so we want setters on params?

    //  Generates a saw wave from 0..1
    static T runSaw(SawOscillatorState<T>& state, const SawOscillatorParams<T>& params);

    //  Generates a triangle wave from -1..1
    static T runTri(SawOscillatorState<T>& state, const SawOscillatorParams<T>& params);

    /**
     * gets the regular output and the quadrature output
     */
    static void runQuadrature(T& out, T& outQuad, SawOscillatorState<T>& state, const SawOscillatorParams<T>& params);
};

template<typename T, bool frequencyCanBeNegative>
inline T SawOscillator<T, frequencyCanBeNegative>::runSaw(SawOscillatorState<T>& state, const SawOscillatorParams<T>& params)
{
    T ret = state.phase;
    state.phase += params.phaseIncrement;
    if (state.phase >= 1) {
        state.phase -= 1;
    }

    if (frequencyCanBeNegative && (state.phase < 0)) {
        state.phase += 1;
    }

    return ret;
}


template<typename T, bool frequencyCanBeNegative>
inline T SawOscillator<T, frequencyCanBeNegative>::runTri(SawOscillatorState<T>& state, const SawOscillatorParams<T>& params)
{
    T output = 4 * runSaw(state, params);    // Saw 0...4
  //  printf("in tri, 4x = %f ", output);
    if (output > 3) {
        output -= 4;
    } else if (output > 1) {
        output = 2 - output;
    }
  //  printf("final = %f \n", output);
    return output;
}

template<typename T, bool frequencyCanBeNegative>
inline void SawOscillator<T, frequencyCanBeNegative>::runQuadrature(T& out, T& outQuad, SawOscillatorState<T>& state, const SawOscillatorParams<T>& params)
{
    out = runSaw(state, params);
    T quad = out + T(.25);
    if (quad >= 1) {
        quad -= 1;
    }
    outQuad = quad;
}



template<typename T, bool frequencyCanBeNegative>
inline void SawOscillator<T, frequencyCanBeNegative>::setFrequency(SawOscillatorParams<T>& params, T freq)
{
    if (frequencyCanBeNegative) {
        assert(freq >= -.5 && freq <= .5);
    } else {
        assert(freq >= 0 && freq <= .5);
    }
    params.phaseIncrement = freq;
}

template<typename T>
class SawOscillatorParams
{
public:
    T phaseIncrement = 0;
};

template<typename T>
class SawOscillatorState
{
public:
    /**
    * phase increments from 0...1
    */
    T phase = 0;
};
