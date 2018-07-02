
#include "HilbertFilterDesigner.h"
#include "AudioMath.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"

#include <cmath>
#include <algorithm>
#include <assert.h>

// test that we can hook it up
template<typename T>
static void test0()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);
}

// test that filter designer does something (more than just generate zero
template<typename T>
static void test1()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    const double delta = .00001;
    for (int i = 0; i < 3; ++i) {
        assert(!AudioMath::closeTo(0, (paramsSin.A1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.A2(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B0(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B2(i)), delta));

        assert(!AudioMath::closeTo(0, (paramsCos.A1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.A2(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B0(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B2(i)), delta));
    }
}

// see if it passes audio
template <typename T>
static void test2()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    BiquadState<T, 3> stateSin;
    BiquadState<T, 3> stateCos;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    //  const T hilbertSin = BiquadFilter<T>::run(input, hilbertFilterStateSin, hilbertFilterParamsSin);
    T input = 1;
  //  T t = BiquadFilter<T>::run(input, state, paramsSin);
    for (int i = 0; i < 10; ++i) {
        T ts = BiquadFilter<T>::run(input, stateSin, paramsSin);
        T tc = BiquadFilter<T>::run(input, stateCos, paramsCos);
        assert(!AudioMath::closeTo(ts, 0, .00001));
        assert(!AudioMath::closeTo(tc, 0, .00001));
    }
}

template<typename T>
static void test()
{
    test0<T>();
    test1<T>();
    test2<T>();

}

void testHilbert()
{
    test<double>();
    test<float>();
}