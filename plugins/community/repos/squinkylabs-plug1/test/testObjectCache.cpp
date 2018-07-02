
#include "asserts.h"
#include "ObjectCache.h"

extern int _numLookupParams;

template <typename T>
static void testBipolar()
{
    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache<T>::getBipolarAudioTaper();
    assertEQ(_numLookupParams, 1);
    auto test2 = ObjectCache<T>::getBipolarAudioTaper();
    assertEQ(_numLookupParams, 1);
    test.reset();
    assertEQ(_numLookupParams, 1);

    test2.reset();
    assertEQ(_numLookupParams, 0);

    {
        // simple test that bipolar audio scalers use cached lookups, and they work
        AudioMath::ScaleFun<float> f = AudioMath::makeBipolarAudioScaler(3, 4);
        assertEQ(f(0, -5, 0), 3.);
        assertEQ(_numLookupParams, 1);
    }
    assertEQ(_numLookupParams, 0);

    // make again
    test = ObjectCache<T>::getBipolarAudioTaper();
    assertEQ(_numLookupParams, 1);
}

template <typename T>
static void testSin()
{
    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache<T>::getSinLookup();
    assertEQ(_numLookupParams, 1);
    auto test2 = ObjectCache<T>::getSinLookup();
    assertEQ(_numLookupParams, 1);
    test.reset();
    assertEQ(_numLookupParams, 1);

    test2.reset();
    assertEQ(_numLookupParams, 0);

    {
     //   // simple test that bipolar audio scalers use cached lookups, and they work
        AudioMath::ScaleFun<float> f = AudioMath::makeBipolarAudioScaler(3, 4);
        assertEQ(f(0, -5, 0), 3.);
        assertEQ(_numLookupParams, 1);
    }
    assertEQ(_numLookupParams, 0);

    // make again
    test = ObjectCache<T>::getSinLookup();
    assertEQ(_numLookupParams, 1);
}


template <typename T>
static void testExp2()
{
    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache<T>::getExp2();
    assertEQ(_numLookupParams, 1);
    auto test2 = ObjectCache<T>::getExp2();
    assertEQ(_numLookupParams, 1);
    test.reset();
    assertEQ(_numLookupParams, 1);

    test2.reset();
    assertEQ(_numLookupParams, 0);

    {
        auto test3 = ObjectCache<T>::getExp2();
        const double x = LookupTable<T>::lookup(*test3, (T)3.2);
        const double y = std::pow(2, 3.2);
        assertClose(x, y, .001);
        assertEQ(_numLookupParams, 1);
    }
    assertEQ(_numLookupParams, 0);

    // make again
    test = ObjectCache<T>::getExp2();
    assertEQ(_numLookupParams, 1);
}

template <typename T>
static void testExp2b()
{
    {
    // make sure exp2 is really 1V/octave
        auto ex2 = ObjectCache<T>::getExp2();
        const T a = LookupTable<T>::lookup(*ex2, 5);
        const T b = LookupTable<T>::lookup(*ex2, 6);
        assertClose(b / a, 2, .001);
    }
}


template <typename T>
static void testDb2Gain()
{
    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache<T>::getDb2Gain();
    assertEQ(_numLookupParams, 1);
    auto test2 = ObjectCache<T>::getDb2Gain();
    assertEQ(_numLookupParams, 1);
    test.reset();
    assertEQ(_numLookupParams, 1);

    test2.reset();
    assertEQ(_numLookupParams, 0);

    {
        auto test3 = ObjectCache<T>::getDb2Gain();
        const double x = LookupTable<T>::lookup(*test3, (T) -12);
        const double y = AudioMath::gainFromDb(-12);


        assertClose(x, y, .1);
        assertEQ(_numLookupParams, 1);
    }
    assertEQ(_numLookupParams, 0);

    // make again
    test = ObjectCache<T>::getDb2Gain();
    assertEQ(_numLookupParams, 1);
}

template <typename T>
static void testDb2Gain2()
{
    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache<T>::getDb2Gain();

    // .1 db from -80 to +20
    for (double db = -80; db < 20; db += .1) {
        const double x = LookupTable<T>::lookup(*test, (T) db);
        const double y = AudioMath::gainFromDb(db);

        assertClose(x, y, .2);
    }
}


template <typename T>
static void testTanh5()
{
    auto test = ObjectCache<T>::getTanh5();
    auto test2 = ObjectCache<T>::getTanh5();
    assertEQ(_numLookupParams, 1);

    for (double x = -5; x <= 5; x += .1) {
        const double y = LookupTable<T>::lookup(*test, (T) x);
        const double y0 = std::tanh(x);

        assertClose(y, y0, .01);
    }
}

template <typename T>
static void test()
{
    testBipolar<T>();
    testSin<T>();
    testExp2<T>();
    testExp2b<T>();
    testDb2Gain<T>();
    testDb2Gain2<T>();
    testTanh5<T>();
}

void testObjectCache()
{
    assertEQ(_numLookupParams, 0);
    test<float>();
    test<double>();
    assertEQ(_numLookupParams, 0);
}