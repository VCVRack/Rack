
#include <assert.h>
#include <iostream>

#include "asserts.h"
#include "AudioMath.h"
#include "LookupTable.h"
#include "LookupTableFactory.h"

using namespace std;

// test that we can call all the functions
template<typename T>
static void test0()
{
    LookupTableParams<T> p;
    const int tableSize = 512;

    std::function<double(double)> f = [](double d) {
        return 0;
    };

    LookupTable<T>::init(p, tableSize, 0, 1, f);
    LookupTable<T>::lookup(p, 0);
}

// test that simple lookup works
template<typename T>
static void test1()
{
    LookupTableParams<T> p;
    const int tableSize = 512;

    std::function<double(double)> f = [](double d) {
        return 100;
    };

    LookupTable<T>::init(p, tableSize, 0, 1, f);
    assert(LookupTable<T>::lookup(p, 0) == 100);
    assert(LookupTable<T>::lookup(p, 1) == 100);
    assert(LookupTable<T>::lookup(p, T(.342)) == 100);
}


// test that sin works
template<typename T>
static void test2()
{
    LookupTableParams<T> p;
    const int tableSize = 512;

    std::function<double(double)> f = [](double d) {
        return std::sin(d);
    };

    LookupTable<T>::init(p, tableSize, 0, 1, f);

    const T tolerance = T(0.000001);
    for (double d = 0; d < 1; d += .0001) {
        T output = LookupTable<T>::lookup(p, T(d));

        const bool t = AudioMath::closeTo(output, std::sin(d), tolerance);
        if (!t) {
            cout << "failing with expected=" << std::sin(d) << " actual=" << output << " delta=" << std::abs(output - std::sin(d));
            assert(false);
        }
    }
}


// test that sin works on domain 10..32
template<typename T>
static void test3()
{
    LookupTableParams<T> p;
    const int tableSize = 16;

    std::function<double(double)> f = [](double d) {
        const double s = (d - 10) / 3;
        return std::sin(s);
    };

    LookupTable<T>::init(p, tableSize, 10, 13, f);

    const T tolerance = T(0.01);
    for (double d = 10; d < 13; d += .0001) {
        const T output = LookupTable<T>::lookup(p, T(d));

        const T expected = (T) std::sin((d - 10.0) / 3);
        const bool t = AudioMath::closeTo(output, expected, tolerance);
        if (!t) {
            cout << "failing with d=" << d << " expected=" << expected << " actual=" << output << " delta=" << std::abs(output - std::sin(d));
            assert(false);
        }
    }
}

// test that sin at extremes works
template<typename T>
static void test4()
{
    LookupTableParams<T> exponential;

    const T xMin = -5;
    const T xMax = 5;

    std::function<double(double)> expFunc = AudioMath::makeFunc_Exp(-5, 5, 2, 2000);
    LookupTable<T>::init(exponential, 128, -5, 5, expFunc);

    // Had to loosen tolerance to pass with windows gcc. Is there a problem
    // with precision, or is this expected with fast math?
    const T tolerance = T(0.0003);
    T outputLow = LookupTable<T>::lookup(exponential, xMin);

    T outputHigh = LookupTable<T>::lookup(exponential, xMax);

    bool t = AudioMath::closeTo(outputLow, 2, tolerance);
    if (!t) {
        cout << "failing l with expected=" << 2 << " actual=" << outputLow << " delta=" << std::abs(outputLow - 2) << std::endl;
        assert(false);
    }
    t = AudioMath::closeTo(outputHigh, 2000, tolerance);
    if (!t) {
        cout << "failing h with expected=" << 2000 << " actual=" << outputHigh << " delta=" << std::abs(outputHigh - 2000) << std::endl;
        assert(false);
    }
}

template<typename T>
static void testDiscrete1()
{
    LookupTableParams<T> lookup;

    T y[] = {0, 10};
    LookupTable<T>::initDiscrete(lookup, 2, y);

    assertEQ(LookupTable<T>::lookup(lookup, 0), 0);
    assertEQ(LookupTable<T>::lookup(lookup, .5), 5);
    assertEQ(LookupTable<T>::lookup(lookup, 1), 10);

    assertEQ(LookupTable<T>::lookup(lookup, T(.1)), 1);
    assertClose(LookupTable<T>::lookup(lookup, T(.01)), T(.1), .00001);
}

template<typename T>
static void testDiscrete2()
{
    LookupTableParams<T> lookup;

    T y[] = {100, 100.5, 2000, -10};
    LookupTable<T>::initDiscrete(lookup, 4, y);

    assertEQ(LookupTable<T>::lookup(lookup, 0), 100);
    assertEQ(LookupTable<T>::lookup(lookup, 1), 100.5);
    assertEQ(LookupTable<T>::lookup(lookup, 2), 2000);
    assertEQ(LookupTable<T>::lookup(lookup, 3), -10);

    assertEQ(LookupTable<T>::lookup(lookup, 2.5), 1000 - 5);
}

template<typename T>
static void testExpSimpleLookup()
{
    LookupTableParams<T> lookup;
    LookupTableFactory<T>::makeExp2(lookup);

    const double xMin = LookupTableFactory<T>::expXMin();
    const double xMax = LookupTableFactory<T>::expXMax();
    assert(5 > xMin);
    assert(11 < xMax);
    assertClose(LookupTable<T>::lookup(lookup, 5), std::pow(2, 5), .01);
    assertClose(LookupTable<T>::lookup(lookup, 11), std::pow(2, 11), 2);        // TODO: tighten
}



// test that extreme inputs is clamped
template<typename T>
static void testExpRange()
{
    LookupTableParams<T> lookup;
    LookupTable<T>::makeExp2(lookup);
    auto k1 = LookupTable<T>::lookup(lookup, -1);
    auto k2 = LookupTable<T>::lookup(lookup, 11);

    assertClose(LookupTable<T>::lookup(lookup, -1), LookupTable<T>::lookup(lookup, 0), .01);
    assertClose(LookupTable<T>::lookup(lookup, 11), LookupTable<T>::lookup(lookup, 10), .01);
    assertClose(LookupTable<T>::lookup(lookup, -100), LookupTable<T>::lookup(lookup, 0), .01);
    assertClose(LookupTable<T>::lookup(lookup, 1100), LookupTable<T>::lookup(lookup, 10), .01);
}


template<typename T>
static void testExpTolerance(T centsTolerance)
{
    const T xMin = (T) LookupTableFactory<T>::expXMin();
    const T xMax = (T) LookupTableFactory<T>::expXMax();

    LookupTableParams<T> table;
    LookupTableFactory<T>::makeExp2(table);
    for (T x = xMin; x <= xMax; x += T(.0001)) {
        T y = LookupTable<T>::lookup(table, x);            // and back
        double accurate = std::pow(2.0, x);
        double errorCents = std::abs(1200.0 * std::log2(y / accurate));
        assertClose(errorCents, 0, centsTolerance);
    }
}

template <typename T>
static void testBipolarSimpleLookup()
{
    LookupTableParams<T> lookup;
    LookupTableFactory<T>::makeBipolarAudioTaper(lookup);

    assertClose(LookupTable<T>::lookup(lookup, 0), 0, .01);
    assertClose(LookupTable<T>::lookup(lookup, 1), 1, .01);
    assertClose(LookupTable<T>::lookup(lookup, -1), -1, .01);
}


template <typename T>
static void testBipolarTolerance()
{
    LookupTableParams<T> lookup;
    LookupTableFactory<T>::makeBipolarAudioTaper(lookup);
    const double toleratedError = 1 - AudioMath::gainFromDb(-.1);// let's go for one db.
    assert(toleratedError > 0);

    auto refFuncPos = AudioMath::makeFunc_AudioTaper(LookupTableFactory<T>::audioTaperKnee());
    auto refFuncNeg = [refFuncPos](double x) {
        assert(x <= 0);
        return -refFuncPos(-x);
    };

    for (double x = -1; x < 1; x += .001) {

        const T test = LookupTable<T>::lookup(lookup, (T) x);
        T ref = 1234;
        if (x < 0) {
            ref = (T) refFuncNeg(x);
        } else {
            ref = (T) refFuncPos(x);
        }
        assertClose(test, ref, toleratedError);
    }
}

template<typename T>
static void test()
{
    test0<T>();
    test1<T>();
    test2<T>();
    test3<T>();
    test4<T>();
    testDiscrete1<T>();
    testDiscrete2<T>();
    testExpSimpleLookup<T>();
    testExpTolerance<T>(100);   // 1 semitone
    testExpTolerance<T>(10);
    testExpTolerance<T>(1);
    testBipolarSimpleLookup<T>();
    testBipolarTolerance<T>();
}

void testLookupTable()
{
    test<float>();
    test<double>();
}