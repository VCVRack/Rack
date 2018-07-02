#include <assert.h>

#include "asserts.h"
#include "StateVariableFilter.h"
#include "TestSignal.h"

/**
 * Simple test - can we get output that looks kind of low pass
 */
template <typename T>
static void test1()
{
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(T(.05));                   // TODO: make an even fraction
    params.setQ(T(.7));

    T lastValue = -1;
    for (int i = 0; i < 5; ++i) {
        T output = StateVariableFilter<T>::run(1, state, params);
        assert(output > lastValue);
        lastValue = output;
    }
}


/**
 * Measure freq response at some points
 */
template <typename T>
static void testLowpass()
{
    const T fc = T(.001);
    const T q = T(1.0 / std::sqrt(2));      // butterworth
    StateVariableFilterParams<T> params;
    StateVariableFilterState<T> state;
    params.setMode(StateVariableFilterParams<T>::Mode::LowPass);
    params.setFreq(fc);                     // TODO: make an even fraction
    params.setQ(q);

    double g = TestSignal<T>::measureGain(fc / 4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, 0, .05));

    g = TestSignal<T>::measureGain(fc, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g = AudioMath::db(g);
    assert(AudioMath::closeTo(g, -3, .05));

    double g2 = TestSignal<T>::measureGain(fc * 4, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g2 = AudioMath::db(g2);

    double g3 = TestSignal<T>::measureGain(fc * 8, [&state, &params](T input) {
        return StateVariableFilter<T>::run(input, state, params);
        });
    g3 = AudioMath::db(g3);
    assert(AudioMath::closeTo(g2 - g3, 12, 2));
}

/**
 * Verify that passband gain tracks Q
 */
static void testBandpass()
{
    const float fc = .01f;
    const float q = (1.0f / float(std::sqrt(2)));      // butterworth
    StateVariableFilterParams<float> params;
    StateVariableFilterState<float> state;
    params.setMode(StateVariableFilterParams<float>::Mode::BandPass);
    params.setFreq(fc);                     // TODO: make an even fraction
    params.setQ(q);

    double g0 = TestSignal<float>::measureGain(fc, [&state, &params](float input) {
        return StateVariableFilter<float>::run(input, state, params);
        });
    g0 = AudioMath::db(g0);

    for (int i = 2; i < 100; i *= 2) {
        const float q = float(i);
        params.setQ(q);

        double g = TestSignal<float>::measureGain(fc, [&state, &params](float input) {
            return StateVariableFilter<float>::run(input, state, params);
            });
        g = AudioMath::db(g);
       // printf("q = %f, gain db = %f qdb=%f\n", q, g, AudioMath::db(q));

        assertClose(g, AudioMath::db(q), .5);
    }
}

template <typename T>
static void testSetBandwidth()
{
    StateVariableFilterParams<T> params;
    params.setNormalizedBandwidth(T(.1));
    assertEQ(params.getNormalizedBandwidth(), T(.1));

    params.setNormalizedBandwidth(T(.5));
    assertEQ(params.getNormalizedBandwidth(), T(.5));

    params.setQ(10);
    assertEQ(params.getNormalizedBandwidth(), T(.1))
}

template <typename T>
static void test()
{
    test1<T>();
    testLowpass<T>();
    testSetBandwidth<T>();
}

void testStateVariable()
{
    test<float>();
    test<double>();
    testBandpass();
}