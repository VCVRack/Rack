#include <assert.h>
#include <iostream>

#include "MultiModOsc.h"
#include "AudioMath.h"
#include "SawOscillator.h"
#include "TestSignal.h"

using namespace std;

// do objects exist?
template<typename T>
static void testSaw1()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, false>::setFrequency(params, (T(.1)));
    SawOscillatorState<T> state;
    SawOscillator<T, false>::runSaw(state, params);

    using osc = MultiModOsc<T, 4, 3>;
    typename osc::State mstate;
    typename osc::Params mparams;
    T output[3];
    osc::run(output, mstate, mparams);
}

/**
 * Does parameter calculation do anything?
 */
template<typename T>
static void testSaw2()
{
    SawOscillatorParams<T> params;
    assert(params.phaseIncrement == 0);
    SawOscillator<T, false>::setFrequency(params, (T(.1)));
    assert(params.phaseIncrement > 0);
}

/**
 * Does something come out?
 */
template<typename T>
static void testSaw3()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.2)));
    SawOscillatorState<T> state;
    SawOscillator<T, true>::runSaw(state, params);
    const T out = SawOscillator<T, true>::runSaw(state, params);
    assert(out > 0);
    assert(out < 1);
}

/**
* Does something come out?
*/
template<typename T>
static void testTri3()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.2)));
    SawOscillatorState<T> state;
    SawOscillator<T, true>::runSaw(state, params);
    const T out = SawOscillator<T, true>::runTri(state, params);
    assert(out > 0);
    assert(out < 1);
}

/**
* Does something come out?
*/
template<typename T>
static void testMulti3()
{
    using osc = MultiModOsc<T, 4, 3>;
    typename osc::State state;
    typename osc::Params params;
    T output[3] = {0, 0, 0};
    T output2[3] = {0, 0, 0};

    osc::run(output, state, params);
    osc::run(output, state, params);
    osc::run(output2, state, params);
    for (int i = 0; i < 3; ++i) {
        assert(output[i] != 0);
        assert(output2[i] != 0);
        assert(output[i] != output2[i]);

        if (i > 0) {
            assert(output[i] != output[i - 1]);
            assert(output2[i] != output2[i - 1]);
        }
    }
}

/**
 * Does it look like a triangle?
 */

template<typename T>
static void testTri4()
{
    const T freq = T(.1);     // was .01
    const T delta = 2 * freq;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.01)));
    SawOscillatorState<T> state;

    T last = -freq;
    bool increasing = true;
    for (int i = 0; i < 1000; ++i) {
        const T output = SawOscillator<T, true>::runTri(state, params);

        assert(output >= -1);
        assert(output <= 1);
        if (increasing) {
            if (output > last) {
                // still increasing
            } else {
                // started decreasing

                assert(AudioMath::closeTo(output, 1, delta));
                increasing = false;
            }
        } else {
            if (output < last) {
                // still decreasing
            } else {
                // started increasing
                assert(AudioMath::closeTo(output, -1, delta));
                increasing = true;
            }
        }

        last = output;
    }
}

/**
* Does it look like a saw?
*/
template<typename T>
static void testSaw4()
{
    const T freq = T(.01);
    const T delta = freq / 1000;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.01)));
    SawOscillatorState<T> state;

    T last = 0;
    for (int i = 0; i < 1000; ++i) {
        const T output = SawOscillator<T, true>::runSaw(state, params);

        assert(output >= 0);
        assert(output < 1);

        if (output < last) {
            assert(last > .99);
            assert(output < .01);
        } else {
            assert(output < (last + freq + delta));
        }

        last = output;
    }
}

/**
* Is the quadrature really 90 out of phase?
*/
template<typename T>
static void testSaw5()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.01)));
    SawOscillatorState<T> state;

    T output;
    T quadratureOutput;
    for (int i = 0; i < 1000; ++i) {
        SawOscillator<T, true>::runQuadrature(output, quadratureOutput, state, params);

        // normalize output (unwrap)
        if (quadratureOutput < output) {
            quadratureOutput += 1;
        }
        assert(quadratureOutput = (output + T(.25)));
    }
}


/**
* Does it look like a negative saw?
*/
template<typename T>
static void testSaw6()
{
    const T freq = T(-.01);
    const T delta = freq / 1000;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, freq);
    SawOscillatorState<T> state;

    T last = 0;
    for (int i = 0; i < 1000; ++i) {
        const T output = SawOscillator<T, true>::runSaw(state, params);

        assert(output >= 0);
        assert(output < 1);

        if (output > last) {
            // wrap case - did we more or less wrap?
            assert(last < .01);
            assert(output > .98);
        } else {
            // no-wrap - are we decreasing
            assert(output > (last + freq + delta));
        }
        last = output;
    }
}

/**
 * IS the RMS for triangle as expected?
 */
template <typename T>
static void testTri7()
{
    const int div = 1024;
    const T freq = T(1.0 / T(div));
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, freq);
    SawOscillatorState<T> state;
    double amplitude = TestSignal<T>::measureOutput(div, [&state, &params]() {
        return SawOscillator<T, true>::runTri(state, params);
        });

    // RMS of tri wave is 1 / cube root 3
    assert(AudioMath::closeTo(amplitude, 0.57735, .0001));
}

template <typename T>
static void testSaw7()
{
    const int div = 1024;
    const T freq = T(1.0 / T(div));
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, freq);
    SawOscillatorState<T> state;
    double amplitude = TestSignal<T>::measureOutput(div * 16, [&state, &params]() {
        // normalize to 1V pp
        return 2 * SawOscillator<T, true>::runSaw(state, params) - 1;
        });


    // RMS of saw wave is 1 / cube root 3
    assert(AudioMath::closeTo(amplitude, 0.57735, .0001));
}

template <typename T>
static void testSawT()
{
    testSaw1<T>();
    testSaw2<T>();
    testSaw3<T>();
    testTri3<T>();
    testMulti3<T>();
    testSaw4<T>();
    testTri4<T>();
    testSaw5<T>();
    testSaw6<T>();
    testTri7<T>();
    testSaw7<T>();
}

void testSaw()
{
    testSawT<float>();
    testSawT<double>();
}