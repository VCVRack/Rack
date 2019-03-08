
#include "asserts.h"
#include "MultiLag.h"
#include "LowpassFilter.h"
#include "Decimator.h"
#include "Analyzer.h"
#include "asserts.h"
#include "LFN.h"
#include "TestComposite.h"


template<typename T>
static void test0()
{
    LowpassFilterState<T> state;
    LowpassFilterParams<T> params;
    LowpassFilter<T>::setCutoff(params, T(.1));
    LowpassFilter<T>::run(0, state, params);
}

const float sampleRate = 44100;
template<typename T>
static void doLowpassTest(std::function<float(float)> filter, T Fc, T expectedSlope)
{
    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);

    const double cutoff = FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples);

    // Is the peak at zero? i.e. no ripple.
    if (std::get<1>(x) == 0) {
        assertEQ(std::get<0>(x), -1);   // no LF shoulder
    } else {
        // Some higher order filters have a tinny bit of ripple
        float peakMag = std::abs(response.get(std::get<1>(x)));
        float zeroMag = std::abs(response.get(0));
        assertClose(peakMag / zeroMag, 1, .0001);
    }

    assertClose(cutoff, Fc, 3);    // 3db down at Fc

    double slope = Analyzer::getSlope(response, (float) Fc * 2, sampleRate);
    assertClose(slope, expectedSlope, 1);          // to get accurate we need to go to higher freq, etc... this is fine
}


template<typename T>
static void test1()
{
    const float Fc = 100;

    LowpassFilterState<T> state;
    LowpassFilterParams<T> params;
    LowpassFilter<T>::setCutoff(params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = LowpassFilter<T>::run(x, state, params);
        return float(y);
    };
    doLowpassTest<T>(filter, Fc, -6);
}

template<typename T>
static void test2()
{
    const float Fc = 100;
    BiquadParams<T, 1> params;
    BiquadState<T, 1> state;

    ButterworthFilterDesigner<T>::designTwoPoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doLowpassTest<T>(filter, Fc, -12);
}

template<typename T>
static void test3()
{
    const float Fc = 100;
    BiquadParams<T, 2> params;
    BiquadState<T, 2> state;

    ButterworthFilterDesigner<T>::designThreePoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doLowpassTest<T>(filter, Fc, -18);
}

template<typename T>
static void test4()
{
    const float Fc = 100;
    BiquadParams<T, 2> params;
    BiquadState<T, 2> state;

    ButterworthFilterDesigner<T>::designFourPoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doLowpassTest<T>(filter, Fc, -24);
}
template<typename T>
static void test5()
{
    const float Fc = 100;
    BiquadParams<T, 3> params;
    BiquadState<T, 3> state;

    ButterworthFilterDesigner<T>::designFivePoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doLowpassTest<T>(filter, Fc, -30);
}

template<typename T>
static void test6()
{
    const float Fc = 100;
    BiquadParams<T, 3> params;
    BiquadState<T, 3> state;

    ButterworthFilterDesigner<T>::designSixPoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doLowpassTest<T>(filter, Fc, -36);
}

/******************************************************************************************************/

#if 0 // not ready for prime time
template<typename T>
static void doEllipticTest(std::function<float(float)> filter, T Fc, T expectedSlope)
{
    const int numSamples = 16 * 1024;
   //const int numSamples = 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);

    const float cutoff = FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples);


    // Some higher order filters have a tinny bit of ripple
    float peakMag = std::abs(response.get(std::get<1>(x)));
    float zeroMag = std::abs(response.get(0));
    printf("mag at zero hz = %f, peak mag = %f, -3db at %f\n ",
        zeroMag, peakMag, cutoff);

    Analyzer::getAndPrintFeatures(response, 1, sampleRate);
    for (int i = 0; i < numSamples; ++i) {

    }
    //assertClose(peakMag / zeroMag, 1, .0001);


  //  assertClose(cutoff, Fc, 3);    // 3db down at Fc


    double slope = Analyzer::getSlope(response, (float) Fc * 2, sampleRate);
  //  assertClose(slope, expectedSlope, 1);          // to get accurate we need to go to higher freq, etc... this is fine

}

template<typename T>
static void testElip1()
{
    const float Fc = 100;
    BiquadParams<T, 4> params;
    BiquadState<T, 4> state;

    T rippleDb = 3;
    T attenDb = 100000;
    T ripple = (T) AudioMath::gainFromDb(1);
    ButterworthFilterDesigner<T>::designEightPoleElliptic(params, Fc / sampleRate, rippleDb, attenDb);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<T>::run(x, state, params);
        return x;
    };
    doEllipticTest<T>(filter, Fc, -36);

}
#endif

template<typename T>
void _testLowpassFilter()
{
    test0<T>();
    test1<T>();
    test2<T>();
    test3<T>();
    test4<T>();
    test5<T>();
    test6<T>();
}


/*********************************************************************************
**
**              also test decimator here
**
**********************************************************************************/

static void decimate0()
{
    Decimator d;
    d.setDecimationRate(2);
    d.acceptData(5);
    bool b = true;
    auto x = d.clock(b);
    assert(!b);
    assertEQ(x, 5);
}


static void decimate1()
{
    Decimator d;
    d.setDecimationRate(2);
    d.acceptData(5);
    bool b = true;
    auto x = d.clock(b);
    assert(!b);
    assertEQ(x, 5);

    x = d.clock(b);
    assert(b);
    assertEQ(x, 5);
}

void testLowpassFilter()
{
    _testLowpassFilter<float>();
    _testLowpassFilter<double>();
    decimate0();
    decimate1();
}



/*********************************************************************************
**
**              also test MultiLag here
**
**********************************************************************************/

template <class T>
static void _testMultiLag0(int size)
{
    T l;
    for (int i = 0; i < size; ++i) {
        assertClose(l.get(i), 0, .0001);
    }
}

static void testMultiLag0()
{
    _testMultiLag0<MultiLPF<8>>(8);
    _testMultiLag0<MultiLag<8>>(8);
}


// test that output eventually matches input
template <class T>
static void _testMultiLag1(int size, T& dut)
{

    float input[100];
    for (int n = 0; n < size; ++n) {
        assert(n < 100);
        for (int i = 0; i < size; ++i) {
            input[i] = (i == n) ? 1.f : 0.f;
        }
        for (int i = 0; i < 10; ++i) {
            dut.step(input);
        }
        for (int i = 0; i < 8; ++i) {
            const float expected = (i == n) ? 1.f : 0.f;
            assertClose(dut.get(i), expected, .0001);
        }
    }
}

static void testMultiLag1()
{
    MultiLPF<8> f;
    f.setCutoff(.4f);
    _testMultiLag1(8, f);

    MultiLag<8> l;
    l.setAttack(.4f);
    l.setRelease(.4f);
    _testMultiLag1(8, l);
}


// test response
template <class T>
static void _testMultiLag2(int size, T& dut, float f)
{
    for (int n = 0; n < size; ++n) {
        float input[100] = {0};
        std::function<float(float)> filter = [&input, n, &dut](float x) {

            input[n] = x;
            dut.step(input);
            auto y = dut.get(n);
            return float(y);
        };
        doLowpassTest<float>(filter, f, -6);
    }
}

static void testMultiLag2()
{
    MultiLPF<8> f;
    float fC = 10.f;
    f.setCutoff(fC / sampleRate);
    _testMultiLag2(8, f, fC);

    MultiLag<8> l;
    l.setAttack(fC / sampleRate);
    l.setRelease(fC / sampleRate);
    _testMultiLag2(8, l, fC);
}

static void testMultiLagDisable()
{
    MultiLag<8> f;
    float fC = 10.f;
    f.setAttack(fC / sampleRate);
    f.setRelease(fC / sampleRate);

    // when enabled, should lag
    const float buffer[8] = {1,1,1,1,1,1,1,1};
    f.step(buffer);
    for (int i = 0; i < 8; ++i) {
        assertNE(f.get(0), 1);
    }

    f.setEnable(false);
    f.step(buffer);
    for (int i = 0; i < 8; ++i) {
        assertEQ(f.get(0), 1);
    }

}


template <typename T>
static void tlp()
{
    auto params = makeLPFilterL_Lookup<T>();
    assert(params->size() == 6);
}

static void testLowpassLookup()
{
    tlp<float>();
    tlp<double>();
}

static void testLowpassLookup2()
{
    auto params = makeLPFilterL_Lookup<float>();

    for (float f = .1f; f < 100; f *= 1.1f) {
        float fs = f / 44100;
        float l = LowpassFilter<float>::computeLfromFs(fs);
        float l1 = NonUniformLookupTable<float>::lookup(*params, fs);

        float r = l1 / l;
        assertClose(r, 1, .01);
    }

    assert(true);
}

static void testDirectLookup()
{
    auto p = makeLPFDirectFilterLookup<float>(1.f / 44100.f);
    assert(p->numBins_i > 0);
    makeLPFDirectFilterLookup<double>(1.f / 44100.f);
}

static void testDirectLookup2()
{
    auto p = makeLPFDirectFilterLookup<float>(1.f / 44100.f);

    float y = LookupTable<float>::lookup(*p, 0);
    assertEQ(y, 1 - .4f);

    y = LookupTable<float>::lookup(*p, 1);
    assertEQ(y, 1 - .00002f);
}

void testMultiLag()
{
    testLowpassLookup();
    testLowpassLookup2();
    testDirectLookup();
    testDirectLookup2();

    testMultiLag0();
    testMultiLag1();
    testMultiLag2();
    testMultiLagDisable();
}