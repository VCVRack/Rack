/**
 *
 */

#include <stdio.h>
#include <assert.h>

#include "ButterworthFilterDesigner.h"
#include "BiquadFilter.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadState.h"


template<typename T, int N>
static void testState_0()
{
    BiquadState<T, N> p;
    for (int i = 0; i < N; ++i) {
        assert(p.z0(i) == 0);
        assert(p.z1(i) == 0);
    }
    p.z0(0) = 5;
    p.z1(0) = 6;
    assert(p.z0(0) == 5);
    assert(p.z1(0) == 6);

    if (N > 1) {
        p.z0(N - 1) = 55;
        p.z1(N - 1) = 66;
        assert(p.z0(N - 1) == 55);
        assert(p.z1(N - 1) == 66);
    }

    assert(p.z0(0) == 5);
    assert(p.z1(0) == 6);
}

template<typename T, int N>
static void testParam_0()
{

    BiquadParams<T, N> p;
    for (int i = 0; i < N; ++i) {
        assert(p.A2(i) == 0);
        assert(p.A1(i) == 0);
        assert(p.B0(i) == 0);
        assert(p.B1(i) == 0);
        assert(p.B2(i) == 0);
    }

    p.A1(0) = 1;
    p.A2(0) = 2;
    p.B0(0) = 10;
    p.B1(0) = 11;
    p.B2(0) = 12;

    assert(p.A1(0) == 1);
    assert(p.A2(0) == 2);
    assert(p.B0(0) == 10);
    assert(p.B1(0) == 11);
    assert(p.B2(0) == 12);

    if (N > 1) {
        p.A1(N - 1) = 111;
        p.A2(N - 1) = 112;
        p.B0(N - 1) = 1110;
        p.B1(N - 1) = 1111;
        p.B2(N - 1) = 1112;

        assert(p.A1(N - 1) == 111);
        assert(p.A2(N - 1) == 112);
        assert(p.B0(N - 1) == 1110);
        assert(p.B1(N - 1) == 1111);
        assert(p.B2(N - 1) == 1112);
    }

    assert(p.A1(0) == 1);
    assert(p.A2(0) == 2);
    assert(p.B0(0) == 10);
    assert(p.B1(0) == 11);
    assert(p.B2(0) == 12);
}

template<typename T>
static void test2()
{
    BiquadParams<T, 2> p;
    ButterworthFilterDesigner<T>::designThreePoleLowpass(p, T(.1));
    BiquadState<T, 2> s;
    T d = BiquadFilter<T>::run(0, s, p);
    (void) d;
}

// test that filter designer does something (more than just generate zero
template<typename T>
static void testBasicDesigner2()
{
    BiquadParams<T, 1> p;
    ButterworthFilterDesigner<T>::designTwoPoleLowpass(p, T(.1));
    assert(p.A1(0) != 0);
    assert(p.A2(0) != 0);
    assert(p.B1(0) != 0);
    assert(p.B2(0) != 0);
    assert(p.B0(0) != 0);
}

// test that filter designer does something (more than just generate zero
template<typename T>
static void testBasicDesigner3()
{
    BiquadParams<T, 2> p;
    ButterworthFilterDesigner<T>::designThreePoleLowpass(p, T(.1));
    assert(p.A1(0) != 0);
    assert(p.A2(0) != 0);
    assert(p.B1(0) != 0);
    assert(p.B2(0) != 0);
    assert(p.B0(0) != 0);
}

// test that filter does something
template<typename T>
static void testBasicFilter2()
{
    BiquadParams<T, 1> params;
    BiquadState<T, 1> state;
    ButterworthFilterDesigner<T>::designTwoPoleLowpass(params, T(.1));

    T lastValue = -1;

    // the first five values of the step increase
    for (int i = 0; i < 100; ++i) {
        T temp = BiquadFilter<T>::run(1, state, params);
        if (i < 5) {
            // the first 5 are strictly increasing and below 1
            assert(temp < 1);
            assert(temp > lastValue);
        } else if (i < 10) {
            // the next are all overshoot
            assert(temp > 1 && temp < 1.05);
        } else if (i > 50) {
            //settled
            assert(temp > .999 && temp < 1.001);
        }

        lastValue = temp;
    }
    const T val = BiquadFilter<T>::run(1, state, params);
    (void) val;

}

// test that filter does something
template<typename T>
static void testBasicFilter3()
{
    BiquadParams<T, 2> params;
    BiquadState<T, 2> state;
    ButterworthFilterDesigner<T>::designThreePoleLowpass(params, T(.1));

    T lastValue = 1;

    //the first five values of the step decrease (to -1)
    for (int i = 0; i < 100; ++i) {
        T temp = BiquadFilter<T>::run(1, state, params);

        if (i < 6) {
            // the first 5 are strictly increasing and below 1
            assert(temp > -1);
            assert(temp < lastValue);
        } else if (i < 10) {
            // the next are all overshoot
            assert(temp < -1);
            assert(temp > -1.1);
        } else if (i > 400) {
            //settled
            assert(temp < -.999 && temp > -1.001);
        }
        lastValue = temp;
    }
}


void testBiquad()
{
    testState_0<double, 1>();
    testState_0<float, 8>();

    testParam_0<double, 1>();
    testParam_0<float, 8>();

    test2<float>();
    test2<double>();

    testBasicDesigner2<double>();
    testBasicDesigner2<float>();
    testBasicDesigner3<double>();
    testBasicDesigner3<float>();

    testBasicFilter2<double>();
    testBasicFilter2<float>();
    testBasicFilter3<double>();
    testBasicFilter3<float>();

    // TODO: actually measure the freq resp
}