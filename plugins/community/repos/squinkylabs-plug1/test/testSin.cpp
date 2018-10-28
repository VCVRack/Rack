/**
 * THD+Noise tests for various sin generators
 */
#include "Analyzer.h"
#include "SinOscillator.h"

#include <functional>

#if 0
const static int numSamples = 16 * 256 * 1024;
//const static int numSamples = 256;
const double sampleRate = 44100.0;
const double sampleTime = 1.0 / sampleRate;

const double testFreq = 700;



static void testOsc(const std::string& title, double freq, std::function<float(void)> osc)
{
    FFTDataCpx spectrum(numSamples);
    Analyzer::getSpectrum(spectrum, false, osc);

    Analyzer::getAndPrintFeatures(spectrum, 3, sampleRate, -180);

    double signal = 0;
    double noise = 0;
    int matches = 0;
    for (int i = 0; i < numSamples / 2; ++i) {
        const double f = FFT::bin2Freq(i, sampleRate, numSamples);
        const double mag = spectrum.getAbs(i);

       // if (f > 600 && f < 800) printf("%.2f: %f\n", f, mag);
      //  if (mag > .00000001)  printf("%.2f: %e\n", f, mag);
        if (freq == f) {
            ++matches;
            signal += mag;
        } else {
            noise += mag;
        }
    }
    assert(matches == 1);
    assert(noise > 0);
    printf("%s SNR = %f (db)\n", title.c_str(), AudioMath::db(signal / noise));
}

template <typename T>
static void testPhaseAcc()
{
    const double f = Analyzer::makeEvenPeriod(testFreq, sampleRate, numSamples);
    SinOscillatorParams<T> params;
    SinOscillatorState<T> state;

    std::string title = (sizeof(T) > 4) ? "double w lookup" : "float w lookup";
    SinOscillator<T, true>::setFrequency(params, float(f * sampleTime));
    testOsc(title, f, [&state, &params]() {
        return (float) SinOscillator<T, true>::run(state, params);
        });
}


static void testPhaseAcc2()
{
    const double f = Analyzer::makeEvenPeriod(testFreq, sampleRate, numSamples);

    std::string title = "phase acc std::sin";

    double acc = 0;
    testOsc(title, f, [f, &acc]() {
           // return SinOscillator<T, true>::run(state, params);

            const double inc = f * sampleTime;
            acc += inc;
            if (acc > 1) {
                acc -= 1;
            }
            return float(std::sin(acc * AudioMath::Pi * 2));
        });
}

template <typename T>
class Osc2
{
public:
    void setFreq(float t)
    {
        assert(t > 0 && t < .51);

        // orig paper
     //   tapWeight = t * (1.0 / AudioMath::Pi);
      //  tapWeight = t * 2;

        
        const T omega = 2.0 * AudioMath::Pi * t;
        tapWeight = 2 * sin(omega / 2.0);

        // from kvre
        //e= 2*sin(omega/2.0); // omega = 2*pi*freq/sr;
    }
    float run()
    {
        const T x = zX - tapWeight * zY;

        // orig paper
    //    const T y = (tapWeight * zX) + (1 - tapWeight * tapWeight) * zY;

        // smith (seems whack?) but seems to work as well as the above
        const T y = (tapWeight * x) + zY;

        zX = x;
        zY = y;
        return float(zX);
    }

    const double k = sqrt(2);

    T zX = 1.200049 / k;
    T zY = .747444 / k;
    T tapWeight = .1;
};


template <typename T>
class Osc3
{
public:
    void setFreq(float t)
    {
        assert(t > 0 && t < .51);
        //   tapWeight = t * (1.0 / AudioMath::Pi);
        tapWeight = 2 * sin(t);
    }
    float run()
    {
        const T x = zX + tapWeight * zY;
        const T y = -(tapWeight * x) + zY;

        zX = x;
        zY = y;
        return float(zX);
    }

 //   const double k = sqrt(2);

    T zX = 1;
    T zY = 1;
    T tapWeight = .1;
};


static void testOsc2()
{
   const double f = Analyzer::makeEvenPeriod(testFreq, sampleRate, numSamples);
   // const double f = sampleRate / 2;

   printf("trying osc 2 at %f\n", f);
    std::string title = "osc2";
    Osc2<double> osc;
    osc.setFreq(float(f * sampleTime));
    for (int i = 0; i < 1000000; ++i) {
        osc.run();
    }

    testOsc(title, f, [&osc]() {
        return osc.run();
        });
}


static void testOsc3()
{
     const double f = Analyzer::makeEvenPeriod(testFreq, sampleRate, numSamples);
   // const double f = sampleRate / 2;

    std::string title = "osc3";
    Osc3<double> osc;
    osc.setFreq(float(f * sampleTime));
    for (int i = 0; i < 1000000; ++i) {
        osc.run();
    }

    testOsc(title, f, [&osc]() {
        return osc.run();
        });
}

static void testOsc2Amp_sub(double freq)
{
    const int bufSize = 16 * 1024 * 16;
    Osc2<double> osc;
    osc.setFreq(float(freq * sampleTime));

    for (int i = 0; i < 1000000; ++i) {
        osc.run();
    }


    std::vector<float> data;
    data.resize(bufSize);
    for (int i = 0; i < bufSize; ++i) {
        data[i] = osc.run();
    }
    auto minMax = AudioMath::getMinMax<float>(data.data(), bufSize);
    printf("f= %.2f min/max = %f, %f. z = %f, %f\n", freq, minMax.first, minMax.second, osc.zX, osc.zY);
    printf("sqr2 = %f\n", sqrt(2.0));
}

static void testOsc2Amp()
{
    testOsc2Amp_sub(1111);
    testOsc2Amp_sub(40);
    testOsc2Amp_sub(400);
    testOsc2Amp_sub(4123);
}

void testSin()
{
   // testPhaseAcc<float>();
  //  testPhaseAcc<double>();
   testPhaseAcc2();
  // testOsc2();
   // testOsc3();
   // testOsc2Amp();
}
#endif