
#include "Analyzer.h"
#include "asserts.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "ButterworthFilterDesigner.h"
#include "ObjectCache.h"
//#include "FFT.h"

#include <functional>


//std::function<float(float)> filter
const float sampleRate = 44100;

class LowpassStats
{
public:
    double passBandStop = 0;        // -3db point in passband
    double stopBandStart = 0;        // within 3db of stopBandAtten
    double stopBandAttenuation = 0;
};


static LowpassStats characterizeFilter(bool isLowpass, std::function<float(float)> filter, float stopBandGain = -200)
{
    LowpassStats ret;
    bool inPassBand = true;
    bool inStopBand = false;

    const int numSamples = 16 * 1024;
    FFTDataCpx response(numSamples);
    Analyzer::getFreqResponse(response, filter);

    const int passbandBin = isLowpass ? 0 : (numSamples/2) - 1;
    const float gain0 = std::abs(response.get(passbandBin));

    const double _3DbDown = AudioMath::db(gain0) - 3;
    //printf("bin=%d gain=%f\n", passbandBin, gain0);

    assert(gain0 > .1);     // sanity check

    ret.stopBandAttenuation = -1000;
    float minDb = 100;

    const int binInc = isLowpass ? 1 : -1;
 
    for (int i = passbandBin; ; i += binInc) {
        if (i < 0 || i >numSamples / 2) {
            return ret;
        }
        const float db = (float) AudioMath::db(std::abs(response.get(i)));
        const double freq = FFT::bin2Freq(i, sampleRate, numSamples);

        if (inPassBand && db < _3DbDown) {
            ret.passBandStop = freq;
            inPassBand = false;
        }
        // if we bounce back up from min, we are in stop band.
        // only works for Ch I and elliptic
        if (!inPassBand && !inStopBand && (db > (minDb + 3))) {
            inStopBand = true;
            ret.stopBandStart = freq;
        }
        // If we are below something passed in, we are in stop.
        if (!inPassBand && !inStopBand && (db < stopBandGain)) {
            inStopBand = true;
            ret.stopBandStart = freq;
            ret.stopBandAttenuation = db;
        }
        minDb = std::min(minDb, db);
        if (inStopBand) {
            ret.stopBandAttenuation = std::max<double>(ret.stopBandAttenuation, db);
        }
    }

    return ret;

}

static LowpassStats characterizeHighpassFilter(std::function<float(float)> filter, float stopBandGain = -200)
{
    return characterizeFilter(false, filter, stopBandGain);
}

static LowpassStats characterizeLowpassFilter(std::function<float(float)> filter, float stopBandGain = -200)
{
    return characterizeFilter(true, filter, stopBandGain);
}

static void printStats(const char * label, const LowpassStats& stats)
{
    printf("%s band=%.2f,%.2f atten=%.2f\n", label, stats.passBandStop, stats.stopBandStart, stats.stopBandAttenuation);
}

static void testButter4Hi()
{
    const float Fc = 100;
    BiquadParams<float, 2> params;
    BiquadState<float, 2> state;

    ButterworthFilterDesigner<float>::designFourPoleHighpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, params);
        return x;
    };

    const LowpassStats stats = characterizeHighpassFilter(filter, -60);
    assertClose(stats.stopBandStart, 100 / 317, 20);
    assertClose(stats.passBandStop, 100, 5);
#ifdef _LOG
    printStats("butter6/100", stats);
#endif
}



static void testButter6()
{
    const float Fc = 100;
    BiquadParams<float, 3> params;
    BiquadState<float, 3> state;

    ButterworthFilterDesigner<float>::designSixPoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, params);
        return x;
    };

    const LowpassStats stats = characterizeLowpassFilter(filter, -60);
    assertClose(stats.stopBandStart, 317, 20);
    assertClose(stats.passBandStop, 100, 5);
#ifdef _LOG
    printStats("butter6/100", stats);
#endif
}

static void testButter6Obj()
{
    const float fc = sampleRate / 64;
  //  BiquadParams<float, 3> params;
    auto params = ObjectCache<float>::get6PLPParams(1.f / 64.f);
    BiquadState<float, 3> state;

   // ButterworthFilterDesigner<float>::designSixPoleLowpass(
   //     params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, *params);
        return x;
    };

    
    const LowpassStats stats = characterizeLowpassFilter(filter, -60);
    assertClose(stats.stopBandStart, fc * 3.17, 20);
    assertClose(stats.passBandStop, fc, 5);
#ifdef _LOG
    printStats("butter6/obj64", stats);
#endif
}

static void testButter8()
{
    const float Fc = 100;
    BiquadParams<float, 4> params;
    BiquadState<float, 4> state;

    ButterworthFilterDesigner<float>::designEightPoleLowpass(
        params, Fc / sampleRate);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, params);
        return x;
    };

    const LowpassStats stats = characterizeLowpassFilter(filter, -60);
    assertClose(stats.stopBandStart, Fc * 2.5, 20);
    assertClose(stats.passBandStop, Fc, 5);
#ifdef _LOG
    printStats("butter8/100", stats);
#endif
}

static void designSixPoleCheby(BiquadParams<float, 3>& outParams, float frequency, float ripple)
{
    assert(frequency > 0 && frequency < .5);
    using Filter = Dsp::ChebyILowPass<6, 1>;
    Filter f;
    f.SetupAs(frequency, ripple);
    assert(f.GetStageCount() == 3);
    BiquadFilter<float>::fillFromStages(outParams, f.Stages(), f.GetStageCount());
}

static void testCheby6_1()
{
    const float Fc = 100;
    BiquadParams<float, 3> params;
    BiquadState<float, 3> state;

    designSixPoleCheby(params, Fc / sampleRate, 6);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, params);
        return x;
    };

    const LowpassStats stats = characterizeLowpassFilter(filter, -60);
    assertClose(stats.stopBandStart, Fc * 1.6, 20);
    assertClose(stats.passBandStop, Fc, 5);
#ifdef _LOG
    printStats("cheby6/100/6", stats);
#endif
}

static void testCheby6_3()
{
    const float Fc = 100;
    BiquadParams<float, 3> params;
    BiquadState<float, 3> state;

    designSixPoleCheby(params, Fc / sampleRate, 3);

    std::function<float(float)> filter = [&state, &params](float x) {
        x = (float) BiquadFilter<float>::run(x, state, params);
        return x;
    };

    const LowpassStats stats = characterizeLowpassFilter(filter, -60);
    assertClose(stats.stopBandStart, Fc * 1.9, 20);
    assertClose(stats.passBandStop, Fc, 5);
#ifdef _LOG
    printStats("cheby6/100/3", stats);
#endif
}

void testFilterDesign()
{
   
    testButter6();
    testButter4Hi();
    testButter6Obj();
    testButter8();
    testCheby6_1();
    testCheby6_3();
}