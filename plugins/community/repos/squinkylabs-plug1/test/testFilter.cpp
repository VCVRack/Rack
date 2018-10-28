
#include <assert.h>

#include "FFT.h"
#include "GraphicEq.h"
#include "StateVariableFilter.h"
#include "SinOscillator.h"
#include "Analyzer.h"
#include "asserts.h"

static void testPeak(std::function<float(float)> filter, float sampleRate, float expectedMax, float percentTolerance)
{

    const int numSamples = 16 * 1024;
    FFTDataCpx x(numSamples);
    Analyzer::getFreqResponse(x, filter);

    int maxBin = Analyzer::getMax(x);
    double maxFreq = (FFT::bin2Freq(maxBin, 44100, numSamples) +
        FFT::bin2Freq(maxBin + 1, 44100, numSamples)) / 2;

    const float delta = expectedMax * percentTolerance / 100.f;      // One percent accuracy

    // printf("expected: %f actual %f\n", expectedMax, maxFreq);

    assertClose(maxFreq, expectedMax, delta);
}

template <typename T>
static void testBandpass(float Fc, float tolerancePercent)
{
    StateVariableFilterState<T> state;
    StateVariableFilterParams<T> params;

    const float sampleRate = 44100;

    params.setMode(StateVariableFilterParams<T>::Mode::BandPass);
    params.setFreq(Fc / sampleRate);
    params.setQ(3);

    std::function<float(float)> filter = [&state, &params](float x) {
        auto y = StateVariableFilter<T>::run(x, state, params);
        return float(y);
    };
    testPeak(filter, sampleRate, Fc, tolerancePercent);
}

template <typename T>
static void test1()
{
    testBandpass<T>(10, 50);        // coarse FFT resolution
    testBandpass<T>(100, 2);
    testBandpass<T>(1000, 3);
    testBandpass<T>(3000, 12);
    testBandpass<T>(6000, 35);      // This filter just gets inaccurate at high freq - don't know why
}

static void zero(GraphicEq2<5>& geq)
{
    for (int i = 0; i < 5; ++i) {
        geq.setGain(i, 0);
    }
}

static void test2()
{
    const float sampleRate = 44100;
    const int stages = 5;
    GraphicEq2<stages> geq;

    zero(geq);
    geq.setGain(0, 1);
    std::function<float(float)> filter = [&geq](float x) {
        return geq.run(x);
    };
    testPeak(filter, sampleRate, 100, 2);

    zero(geq);
    geq.setGain(1, 1);
    testPeak(filter, sampleRate, 200, 2);

    zero(geq);
    geq.setGain(2, 1);
    testPeak(filter, sampleRate, 400, 2);

    zero(geq);
    geq.setGain(3, 1);
    testPeak(filter, sampleRate, 800, 4);

    zero(geq);
    geq.setGain(4, 1);
    testPeak(filter, sampleRate, 1600, 10);
}


void testFilter()
{
    test1<float>();
    test2();
}