

#include "ButterworthLookup.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "Analyzer.h"
#include "FFT.h"

#include "asserts.h"

// test that the API does something
static void testButterLookup0()
{
    ButterworthLookup4PHP b;
    BiquadParams<float, 2> params;
    for (int i = 0; i < 10; ++i) {
        params.setAtIndex(0, i);
    }
    b.get(params, .1f);
    for (int i = 0; i < 10; ++i) {
        assert(params.getAtIndex(i) != 0);
    }
}

const float sampleRate = 44100;
static void doHighTest(std::function<float(float)> filter, float Fc, float expectedSlope)
{
    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    auto x = Analyzer::getMaxAndShoulders(response, -3);
    printf("max and shoulders return %f, %f, %f\n",
        FFT::bin2Freq(std::get<0>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<1>(x), sampleRate, numSamples),
        FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples)

    );

    const double cutoff = FFT::bin2Freq(std::get<2>(x), sampleRate, numSamples);

    const float highestFreq = sampleRate / 2;
    const int highestBin = numSamples / 2;
    printf("highest freq = %f get0=%d get1=%d\n", highestFreq, std::get<0>(x), std::get<1>(x));
    /*
    test1

    max and shoulders return 99.591064, 21463.220215, -2.691650
    highest freq = 22050.000000 get0=37 get1=7974
    assertClose failed actual value =999.83 expected=1
    Assertion failed: false, file d:\vcvrack\x6\plugins\squinkyvcv\test\testbutterlookup.cpp, line 54

    * 0 = low freq bin #
    * 1 = peak bin #
    * 2 = high bin#
*/


    fflush(stdout);
    // Is the peak at zero? i.e. no ripple.
    if (std::get<1>(x) == highestFreq) {
        assertEQ(std::get<0>(x), -1);   // no LF shoulder
    } else {
        // Some higher order filters have a tinny bit of ripple.
        // make sure that the peak is about the same as the extreme (in this case nyquist
        float peakMag = std::abs(response.get(std::get<1>(x)));
        float maxMag = std::abs(response.get(highestBin));
        assertClose(peakMag / maxMag, 1, .01);
    }

    assertClose(cutoff, Fc, 3);    // 3db down at Fc

    double slope = Analyzer::getSlope(response, (float) Fc * 2, sampleRate);
    assertClose(slope, expectedSlope, 1);          // to get accurate we need to go to higher freq, etc... this is fine
}

static void doHighTest(float fC)
{
    ButterworthLookup4PHP b;
    BiquadParams<float, 2> params;
    BiquadState<float, 2> state;
    b.get(params, fC / sampleRate);

    auto filter = [&state, &params](float x) {
        return BiquadFilter<float>::run(x, state, params);
    };
    doHighTest(filter, fC, 24);
   
}


static void testButterLookup1()
{
    doHighTest(100);
}

void  testButterLookup()
{
    testButterLookup0();
    testButterLookup1();
}