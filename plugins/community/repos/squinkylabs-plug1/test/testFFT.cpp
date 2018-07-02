
#include "asserts.h"
#include <memory>
#include <set>

#include "AudioMath.h"
#include "FFTData.h"
#include "FFT.h"

extern void testFinalLeaks();

static void testAccessors()
{
    FFTDataReal d0(16);

    d0.set(0, 4);
    assertEQ(d0.get(0), 4);

    FFTDataCpx dc(16);

    cpx x(3, 4);
    dc.set(5, x);
    assertEQ(dc.get(5), x);
}

static void testFFTErrors()
{
    FFTDataReal real(16);
    FFTDataCpx cpx(15);
    const bool b = FFT::forward(&cpx, real);
    assert(!b);          // should error if size mismatch
}

static void testForwardFFT_DC()
{
    FFTDataReal real(16);
    FFTDataCpx complex(16);

    // set real for DC
    for (int i = 0; i < 16; ++i) {
        real.set(i, 1.0);
    }
    const bool b = FFT::forward(&complex, real);
    assert(b);

    for (int i = 0; i < 16; ++i) {
        cpx v = complex.get(i);
        float mag = std::abs(v);
        float expect = (i == 0) ? 1.f : 0.f;
        assertEQ(mag, expect);
    }
}

static void test3()
{
    FFTDataReal real(16);
    FFTDataCpx complex(16);

    // set real for fundamental sin.
    // make peak 2.0 so fft will come out to one
    for (int i = 0; i < 16; ++i) {
        auto x = 2.0 *sin(AudioMath::Pi * 2.0 * i / 16.0);
        real.set(i, float(x));
    }
    const bool b = FFT::forward(&complex, real);
    assert(b);

    for (int i = 0; i < 16; ++i) {
        cpx v = complex.get(i);
        float mag = std::abs(v);
      
        float expect = (i == 1) ? 1.f : 0.f;
        assertClose(mag, expect, .0001);
    }
}


static void testRoundTrip()
{
    FFTDataReal realIn(16);
    FFTDataReal realOut(16);
    FFTDataCpx complex(16);

    // set real for DC
    for (int i = 0; i < 16; ++i) {
        realIn.set(i, 1.0);
    }

    bool b = FFT::forward(&complex, realIn);
    assert(b);
    b = FFT::inverse(&realOut, complex);

    for (int i = 0; i < 16; ++i) {
      
        float expect = 1.f; // scaled DC (TODO: fix scaling) 
        assertEQ(realOut.get(i) , expect);
    }
}


static void testNoiseFormula()
{
    const int bins = 64 * 1024 ;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    FFT::makeNoiseSpectrum(data.get(), ColoredNoiseSpec());

    std::set<float> phases;

    for (int i = 0; i < bins; ++i) {
        const cpx x = data->get(i);
        float mag = std::abs(x);
        float phase = std::arg(x);

        const float expectedMag = (i == 0) ? 0.f : (i < (bins / 2)) ? 1.f : 0.f;
        
        assertClose(mag, expectedMag, .0001);
        phases.insert(phase);
    }
}

static float getPeak(const FFTDataReal& data)
{
    float peak = 0;
    for (int i = 0; i < data.size(); ++i) {
        peak = std::max(peak, std::abs(data.get(i)));
    }
    return peak;
}


static void testWhiteNoiseRT()
{
    const int bins = 2048;
    std::unique_ptr<FFTDataCpx> noiseSpectrum(new FFTDataCpx(bins));
    std::unique_ptr<FFTDataReal> noiseRealSignal(new FFTDataReal(bins));
    std::unique_ptr<FFTDataCpx> noiseSpectrum2(new FFTDataCpx(bins));

    for (int i = 0; i < bins; ++i) {
        cpx x(0,0);
        noiseSpectrum2->set(i, x);
    }

    FFT::makeNoiseSpectrum(noiseSpectrum.get(), ColoredNoiseSpec());

    FFT::inverse(noiseRealSignal.get(), *noiseSpectrum);

    FFT::forward(noiseSpectrum2.get(), *noiseRealSignal);
    
    float totalPhase = 0;
    float minPhase = 0;
    float maxPhase = 0;
    for (int i = 0; i < bins/2; ++i) {
        float expected = (i == 0) ? 0.f : 1.f;
        cpx data = noiseSpectrum2->get(i);
      
        assertClose(std::abs(data), expected, .0001);
        const float phase = std::arg(data);
        totalPhase += phase;
        minPhase = std::min(phase, minPhase);
        maxPhase = std::max(phase, maxPhase);

        //printf("phase[%d] = %f\n", i, std::arg(data));
    }
    //printf("TODO: assert on phase\n");
    //printf("total phase=%f, average=%f\n", totalPhase, totalPhase / (bins / 2));
    //printf("maxPhase %f min %f\n", maxPhase, minPhase);
}

static void testNoiseRTSub(int bins)
{
    std::unique_ptr<FFTDataCpx> dataCpx(new FFTDataCpx(bins));
    std::unique_ptr<FFTDataReal> dataReal(new FFTDataReal(bins));
    assertEQ(dataCpx->size(), bins);
    FFT::makeNoiseSpectrum(dataCpx.get(), ColoredNoiseSpec());

    FFT::inverse(dataReal.get(), *dataCpx);
    FFT::normalize(dataReal.get());

    const float peak = getPeak(*dataReal);

    assertClose( peak, 1.0f , .001);

}

static void testNoiseRT()
{
    testNoiseRTSub(4);
    testNoiseRTSub(8);
    testNoiseRTSub(16);
    testNoiseRTSub(1024);
    testNoiseRTSub(1024 * 64);
}


static void testPinkNoise()
{
    const int bins = 1024*4;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    ColoredNoiseSpec spec;
    spec.highFreqCorner = 22100;        // makes no difference for - slope;
    spec.slope = -3;
    spec.sampleRate = 44100;

    FFT::makeNoiseSpectrum(data.get(), spec);


    // pick a starting bin above our 40 hz low freq corner
    const int baseBin = 16;
    //float freqBase = 44100 * baseBin / (float) bins;
    const float freqBase = FFT::bin2Freq(baseBin, 44100, bins);
    assertGT (freqBase, 80);

    // mid-band, quadruple freq should reduce amp by 6db
    float mag16 = std::abs(data->get(baseBin));
    float mag64 = std::abs(data->get(4 * baseBin));

    // TODO: compare in db
    assertClose(mag16, 2 * mag64, .01);


    float lastMag = std::abs(data->get(1));
    for (int i = 1; i < bins / 2; ++i) {
        const float mag = std::abs(data->get(i));
        assertLE(mag, lastMag);
        lastMag = mag;
    }

    for (int i = bins / 2; i < bins; ++i) {
        assertClose(std::abs(data->get(i)), 0, .00001);
    }
}

static void testBlueNoise(float corner = 0)
{
   const int bins = 1024 * 4;
    std::unique_ptr<FFTDataCpx> data(new FFTDataCpx(bins));
    assertEQ(data->size(), bins);

    ColoredNoiseSpec spec;      
    spec.slope = 3;
    spec.sampleRate = 44100;

    if (corner != 0) {
        spec.highFreqCorner = corner;
    } else {
        assertEQ(spec.highFreqCorner, 4000);
    }

    FFT::makeNoiseSpectrum(data.get(), spec);

    float freq16 = 44100 * 16 / (float) bins;
    assertGT(freq16, 20);

    // mid-band, quadruple freq should reduce amp by 6db
    float mag16 = std::abs(data->get(16));
    float mag64 = std::abs(data->get(64));

    assertClose(2 * mag16, mag64, .1);

    float lastMag = 0;
    for (int i = 1; i < bins / 2; ++i) {
        const float mag = std::abs(data->get(i));
        assertGE(mag, .999f * lastMag);
        lastMag = mag;
    }

    for (int i = bins / 2; i < bins; ++i) {
        assertClose(std::abs(data->get(i)), 0, .00001);
    }
}

void testFFT()
{
    assertEQ(FFTDataReal::_count, 0);
    assertEQ(FFTDataCpx::_count, 0);
    testAccessors();
    testFFTErrors();
    testForwardFFT_DC();
    test3();
    testRoundTrip();
    testNoiseFormula();
    testNoiseRT();
    testPinkNoise();
    testBlueNoise();
    testBlueNoise(8000.f);
    testWhiteNoiseRT();
    testFinalLeaks();
}