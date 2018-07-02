#include <assert.h>
#include <iostream>

#include "asserts.h"
#include "SinOscillator.h"
using namespace std;

// test that it can be hooked up
template<typename T>
static void test1()
{
    SinOscillatorParams<T> p;
    SinOscillatorState<T> s;
    SinOscillator<T, true>::setFrequency(p, T(.1));
    T x = SinOscillator<T, true>::run(s, p);
    (void) x;
}

// test that it makes output
template<typename T>
static void test2()
{
    SinOscillatorParams<T> p;
    SinOscillatorState<T> s;
    SinOscillator<T, false>::setFrequency(p, T(.1));
    T x = SinOscillator<T, false>::run(s, p);
    assert(x == 0);
    x = SinOscillator<T, false>::run(s, p);
    assert(x > 0);
}

// test that sin lookup is correct
template<typename T>
static void test3()
{
    SinOscillatorParams<T> params;
    SinOscillatorState<T> s;
    SinOscillator<T, true>::setFrequency(params, T(.1));

    auto& lookup = params.lookupParams;

    const double delta = .00001;

    // sin(0) == 0;
    T y = LookupTable<T>::lookup(*lookup, 0);
    assert(AudioMath::closeTo(y, 0, delta));

    // sin(2pi) == 0
    y = LookupTable<T>::lookup(*lookup, 1);
    assert(AudioMath::closeTo(y, 0, delta));

    // sin(pi/2) == 1
    y = LookupTable<T>::lookup(*lookup, .25);
    assert(AudioMath::closeTo(y, 1, delta));

    // sin(pi) == 0
    y = LookupTable<T>::lookup(*lookup, .5);
    assert(AudioMath::closeTo(y, 0, delta));
}

// test that output is correct freq
template<typename T>
static void test4()
{
    const int clocksPerPeriod = 64;
    SinOscillatorParams<T> params;
    SinOscillatorState<T> state;
    SinOscillator<T, false>::setFrequency(params, T(1.0 / clocksPerPeriod));

    const double delta = .00001;
    T output, quadrature;
    for (int i = 0; i <= clocksPerPeriod; ++i) {
        SinOscillator<T, false>::runQuadrature(output, quadrature, state, params);
        if (i == 0) {
            // sin+cos(0) = 0 + 1
            assert(AudioMath::closeTo(output, 0, delta));
            assert(AudioMath::closeTo(quadrature, 1, delta));
        }
        if (i == 64) {
            // sin+cos(0) = 0 + 1
            assert(AudioMath::closeTo(output, 0, delta));
            assert(AudioMath::closeTo(quadrature, 1, delta));
        }
        if (i == 16) {
            // sin+cos(pi/2) = 1  0
            assert(AudioMath::closeTo(output, 1, delta));
            assert(AudioMath::closeTo(quadrature, 0, delta));
        }
        if (i == 32) {
            // sin+cos(pi) = 0  -1
            assert(AudioMath::closeTo(output, 0, delta));
            assert(AudioMath::closeTo(quadrature, -1, delta));
        }
        if (i == 48) {
            // sin+cos(3pi/2) = -1  0
            assert(AudioMath::closeTo(output, -1, delta));
            assert(AudioMath::closeTo(quadrature, 0, delta));
        }
    }
}

template<typename T>
static void testDistortion()
{
    SinOscillatorParams<T> params;
    SinOscillatorState<T> s;
    SinOscillator<T, true>::setFrequency(params, T(.0423781));
    auto& lookup = params.lookupParams;

    double err = 0;
    for (double d = 0; d < 1; d += .00123) {
        double x = LookupTable<T>::lookup(*lookup, (T) d);
        double y = sin(d * AudioMath::Pi * 2);
      //  printf("d=%f sin=%f look=%f\n", d, y, x);
        assertClose(x, y, .01);

        const double e = std::abs(x - y);
        err = std::max(err, e);
    }
    double errDb = AudioMath::db(err);
   // printf("THD = %f\n", errDb);
    assertLT(errDb, -80);
}

template<typename T>
static void test()
{
    test1<T>();
    test2<T>();
    test3<T>();
    test4<T>();
    testDistortion<T>();
}

void testSinOscillator()
{
    test<double>();
    test<float>();
}