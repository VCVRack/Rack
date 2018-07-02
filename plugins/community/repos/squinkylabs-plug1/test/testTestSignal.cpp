#include <assert.h>
#include <stdio.h>

#include "AudioMath.h"
#include "TestSignal.h"

/**
 * Test that sin generates a signal +-1
 */
template <typename T>
static void test1()
{
    const int size = 20000;
    T buffer[size];
    TestSignal<T>::generateSin(buffer, size, T(.01));

    T min = 1, max = -1;
    for (int i = 0; i < size; ++i) {
        const T x = buffer[i];
        assert(x <= 1);
        assert(x >= -1);
        if (min > x) {
            min = x;
        }
        if (max < x) {
            max = x;
        }
    }

    const T delta = T(.0000001);
    assert(AudioMath::closeTo(min, T(-1), delta));
    assert(AudioMath::closeTo(max, T(1), delta));
}

/**
 * Test the period of sin is correct
 */
template <typename T>
static void test2()
{
    const int size = 20000;
    T buffer[size];
    TestSignal<T>::generateSin(buffer, size, T(.001));

    const T delta = T(.0000001);
    assert(AudioMath::closeTo(buffer[0], 0, delta));

    T last = -1;
    int period = 0;
    for (int i = 0; i < size; ++i) {
        const T x = buffer[i];
        if (x <= last) {
            period = (i - 1) * 4;
            break;
        }
        last = x;
    }
    assert(period == 1000);
}

template <typename T>
static void test3()
{
    const int size = 20000;
    T buffer[size];

    buffer[0] = 1;
    assert(TestSignal<T>::getRMS(buffer, 1) == 1);

    buffer[0] = -1;
    assert(TestSignal<T>::getRMS(buffer, 1) == 1);

    for (int i = 0; i < 5; ++i) {
        buffer[i] = 1;
    }
    assert(TestSignal<T>::getRMS(buffer, 1) == 1);

    for (int i = 0; i < 5; ++i) {
        buffer[i] = -2;
    }
    assert(TestSignal<T>::getRMS(buffer, 1) == 2);

    TestSignal<T>::generateSin(buffer, size, T(.001));
    const double amplitude = TestSignal<T>::getRMS(buffer, size);
    assert(AudioMath::closeTo(amplitude, std::sqrt(2.0) / 2, .00001));
}

template <typename T>
static void testUnityGain()
{
    const T gl = TestSignal<T>::measureGain(T(.001), [](T input) {
        return input;
        });

    assert(gl == 1);
}

template <typename T>
static void test4()
{
    double amp = TestSignal<T>::measureOutput(5, []() {
        return T(-3);
        });
    assert(amp == 3);
}
template <typename T>
static void test()
{
    test1<T>();
    test2<T>();
    test3<T>();
    testUnityGain<T>();
    test4<T>();
}

void testTestSignal()
{
    test<float>();
    test<double>();
}