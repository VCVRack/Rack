
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

/************ also test decimator here
 */

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