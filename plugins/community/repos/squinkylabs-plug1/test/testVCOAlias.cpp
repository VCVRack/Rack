
#include "Analyzer.h"
#include "asserts.h"
#include "EvenVCO.h"
#include "FunVCO.h"
#include "SawOscillator.h"
#include "SinOscillator.h"
#include "TestComposite.h"


// globals for these tests
static const float sampleRate = 44100;
static bool expandBins = false;
static bool adjustBins = true;
#if 0
static const int numSamples = 64 * 1024;
static const double expandThresholdDb = -10;
#else
static const int numSamples = 64 * 1024;
static const double expandThresholdDb = .01;          // normally negative
#endif




static void testPitchQuantize()
{
    const double sampleRate = 44100;
    const int numSamples = 16;
    const double inputFreq = 44100.0 / 4.0;
    double freq = Analyzer::makeEvenPeriod(inputFreq, sampleRate, numSamples);

    // check that quantized pitch is in the bin we expect.
    assertEQ(freq, FFT::bin2Freq(4, sampleRate, numSamples));

    // make saw osc at correct freq
    SinOscillatorParams<double> params;
    SinOscillatorState<double> state;
    SinOscillator<double, false>::setFrequency(params, 1.0 / 4.0);

    // check that spectrum has only a single freq
    std::function<float()> func = [&state, &params]() {
        return float(30 * SinOscillator<double, false>::run(state, params));
    };
    FFTDataCpx spectrum(numSamples);


    Analyzer::getSpectrum(spectrum, false, func);
    for (int i = 0; i < numSamples / 2; ++i) {
        const float abs = spectrum.getAbs(i);
        if (i == 4) {
            assertGE(abs, .5);
        } else {
            assertLT(abs, 0.000000001);
        }
    }
}

class AliasStats
{
public:
    float totalAliasDb;
    float totalAliasAWeighted;
    float maxAliasFreq;
};

/*

Next: examine the spectrum. make sure all freq in spectrum are signal or alias
*/

class FrequencySets
{
public:
    FrequencySets(double fundamental, double sampleRate, const FFTDataCpx& spectrum);
    void expandFrequencies();
    bool checkOverlap() const;

    std::set<double> harmonics;
    std::set<double> alias;

    void adjustFrequencies();
    void dump(const char *) const;
private:
    static void expandFrequencies(std::set<double>&, const FFTDataCpx& spectrum);
    bool adjustFrequencies1();
    static bool adjustFreqHelper(int bin, int tryBin, std::set<double>& set, const FFTDataCpx& spectrum);
    const FFTDataCpx& spectrum;
};

inline void FrequencySets::adjustFrequencies()
{
    int tries = 0;
    while (adjustFrequencies1()) {
        ++tries;
    }
    //printf("adjust moved %d\n", tries);
}

inline bool FrequencySets::adjustFreqHelper(int bin, int tryBin, std::set<double>& set, const FFTDataCpx& spectrum)
{
    bool ret = false;
    if (tryBin < 0 || tryBin >= spectrum.size()) {
        return false;
    }
    const double db = AudioMath::db(spectrum.getAbs(bin));
    const double dbm1 = AudioMath::db(spectrum.getAbs(tryBin));
    if (dbm1 > (db + 3)) {               // only adjust a bin if it's a 3db improvement
        const double f = FFT::bin2Freq(bin, sampleRate, spectrum.size());
        const double fNew = FFT::bin2Freq(tryBin, sampleRate, spectrum.size());
        auto x = set.erase(f);
        assert(x == 1);
        set.insert(fNew);
        ret = true;
    }

    return ret;

}

inline bool FrequencySets::adjustFrequencies1()
{
    for (auto f : harmonics) {
        const int bin = FFT::freqToBin(f, sampleRate, spectrum.size());
        if (adjustFreqHelper(bin, bin - 1, harmonics, spectrum))
            return true;
        if (adjustFreqHelper(bin, bin + 1, harmonics, spectrum))
            return true;
    }
    for (auto f : alias) {
        const int bin = FFT::freqToBin(f, sampleRate, spectrum.size());
        if (adjustFreqHelper(bin, bin - 1, alias, spectrum))
            return true;
        if (adjustFreqHelper(bin, bin + 1, alias, spectrum))
            return true;
    }
    return false;
}


inline FrequencySets::FrequencySets(double fundamental, double sampleRate, const FFTDataCpx& spectrum) :
    spectrum(spectrum)
{
    const double nyquist = sampleRate / 2;
    bool done = false;
    for (int i = 1; !done; ++i) {
        double freq = fundamental * i;
        if (freq < nyquist) {
            //harmonics.push_back(freq);
            harmonics.insert(freq);
        } else {
            double over = freq - nyquist;
            if (over < nyquist) {
                freq = nyquist - over;
                //alias.push_back(freq);
                alias.insert(freq);
            } else {
                done = true;
            }
        }
    }
}


inline void expandHelper(double& maxDb, bool& done, int& i, int deltaI, const FFTDataCpx& spectrum, std::set<double>& f)
{
    if (i >= spectrum.size() || i < 0) {
        done = true;
    } else {
        const double db = AudioMath::db(spectrum.getAbs(i));

        if (db < (maxDb + expandThresholdDb)) {
            done = true;
        } else {
            //const double oldFreq = FFT::bin2Freq(i, sampleRate, spectrum.size());
            const double newFreq = FFT::bin2Freq(i, sampleRate, spectrum.size());
            if (newFreq < 900 && newFreq > 800)
                printf("inserting new freq %f db=%f m=%f\n ", newFreq, db, maxDb);
            maxDb = std::max(maxDb, db);
            f.insert(newFreq);
        }
    }
    i += deltaI;
}

inline void FrequencySets::expandFrequencies(std::set<double>& f, const FFTDataCpx& spectrum)
{
    assert(expandBins);
    for (double freq : f) {
        if (int(freq) == 1064) {
            int x = 5;
        }
        const int bin = FFT::freqToBin(freq, sampleRate, spectrum.size());
        double maxDb = AudioMath::db(spectrum.getAbs(bin));

        // search upward
        bool done;
        int i;
        for (i = bin + 1, done = false; !done; ) {
            expandHelper(maxDb, done, i, 1, spectrum, f);
        }

        for (i = bin - 1, done = false; !done; ) {
            expandHelper(maxDb, done, i, -1, spectrum, f);
        }
    }
}

inline void FrequencySets::dump(const char *label) const
{
    printf("**** %s ****\n", label);
    for (auto f : harmonics) {
        int bin = FFT::freqToBin(f, sampleRate, spectrum.size());
        printf("harm at %.2f db:%.2f\n", f, AudioMath::db(spectrum.getAbs(bin)));
    }
    for (auto f : alias) {
        int bin = FFT::freqToBin(f, sampleRate, spectrum.size());
        printf("alias at %.2f db:%.2f\n", f, AudioMath::db(spectrum.getAbs(bin)));
    }
}

inline void FrequencySets::expandFrequencies()
{
    //dump("before expand freq", spectrum);
    expandFrequencies(harmonics, spectrum);
    expandFrequencies(alias, spectrum);


    //dump("after expand freq", spectrum);
    assert(checkOverlap());

}

inline bool FrequencySets::checkOverlap() const
{
    std::vector<double> overlap;

    std::set_intersection(harmonics.begin(), harmonics.end(),
        alias.begin(), alias.end(),
        std::back_inserter(overlap));
    if (!overlap.empty()) {
        for (auto x : overlap) {
            printf("overlap at %f\n", x);
        }
    }
    return overlap.empty();
}


/*


Ra = 12194**2 * f**4 /
(f**2 + 20.6 ** 2) sqrt((f2 + 107.2**2)(f2 + 737.9**2)) * (f2 + 12194**2)
A(f) = db(Ra) + 2
*/
double WeightA(double mag, double f)
{
    double num = (12194 * 12194) * f*f*f*f;
    double den = (f*f + 20.6*20.6) * sqrt((f*f + 107.2*107.2) * (f*f + 737.9 * 737.9)) * (f*f + 12194 * 12194);
    double Ra = num / den;
   // printf("Ra(%f) = %f\n", f, Ra);
    return Ra * mag;
}

void testAlias(std::function<float()> func, double fundamental, int numSamples)
{
   // printf("test alias fundamental=%f,%f,%f\n", fundamental, fundamental * 2, fundamental * 3);
    FFTDataCpx spectrum(numSamples);
    Analyzer::getSpectrum(spectrum, false, func);
    FrequencySets frequencies(fundamental, sampleRate, spectrum);
    assert(frequencies.checkOverlap());

  // frequencies.dump("init freq");
    if (adjustBins)
        frequencies.adjustFrequencies();
  //  frequencies.dump("after adjust");
    assert(frequencies.checkOverlap());

    if (expandBins)
        frequencies.expandFrequencies();
    assert(frequencies.checkOverlap());

    //frequencies.dump("final freq");

    double totalSignal = 0;
    double totalAlias = 0;
    double totalSignalA = 0;
    double totalAliasA = 0;
    double totalAliasOver5 = 0;
    double totalAliasBelow5 = 0;

    // let's look at every spectrum line
    for (int i = 1; i < numSamples / 2; ++i) {
        const double freq = FFT::bin2Freq(i, sampleRate, numSamples);
        const double mag = spectrum.getAbs(i);
    //    const double db = AudioMath::db(mag);
        const double magA = WeightA(mag, freq);

        const bool isH2 = frequencies.harmonics.find(freq) != frequencies.harmonics.end();
        const bool isA2 = frequencies.alias.find(freq) != frequencies.alias.end();

        assert(!isA2 || !isH2);

        const bool above5k = (freq >= 5000);
     //   const bool above10k = (freq > 10000);

        if (isH2) {
            totalSignal += mag;
            totalSignalA += magA;
        }
        if (isA2) {
            totalAlias += mag;
            totalAliasA += magA;
            if (above5k) {
                totalAliasOver5 += mag;
            } else {
                totalAliasBelow5 += mag;
            }
        }
    }

    printf("total sig = %f alias = %f ratiodb=%f A=%f\n",
        totalSignal,
        totalAlias,
        AudioMath::db(totalAlias / totalSignal),
        2 + AudioMath::db(totalAliasA / totalSignalA)
    );
}

void printHeader(const char * label, double desired, double actual)
{
    printf("\n%s freq = %f, round %f\n", label, desired, actual);
    printf("frame size = %d, expandThreshDb=%f \n", numSamples, expandThresholdDb);
    printf("expand bins=%d, adjustBins=%d\n", expandBins, adjustBins);
}

template <typename T>
void testRawSaw(double normalizedFreq)
{
    const int numSamples = 64 * 1024;
    // adjust the freq to even

    double freq = Analyzer::makeEvenPeriod(sampleRate * normalizedFreq, sampleRate, numSamples);
    printHeader("Raw Saw", sampleRate * normalizedFreq, freq);

    // make saw osc at correct freq
    SawOscillatorParams<T> params;
    SawOscillatorState<T> state;
    SawOscillator<T, false>::setFrequency(params, (float) normalizedFreq);
    testAlias([&state, &params]() {
        return (T) 30 * SawOscillator<T, false>::runSaw(state, params);
        }, freq, numSamples);

}

static void testEven(double normalizedFreq)
{

    // adjust the freq to even
    double freq = Analyzer::makeEvenPeriod(sampleRate * normalizedFreq, sampleRate, numSamples);
    printHeader("EvenVCO", sampleRate * normalizedFreq, freq);

    using EVCO = EvenVCO <TestComposite>;
    EVCO vco;
    vco._testFreq = float(sampleRate * normalizedFreq);
    vco.outputs[EVCO::SAW_OUTPUT].active = true;
    vco.outputs[EVCO::SINE_OUTPUT].active = false;
    vco.outputs[EVCO::TRI_OUTPUT].active = false;
    vco.outputs[EVCO::SQUARE_OUTPUT].active = false;
    vco.outputs[EVCO::EVEN_OUTPUT].active = false;

    testAlias([&vco]() {
        vco.step();
        return 3 * vco.outputs[EVCO::SAW_OUTPUT].value;
        }, freq, numSamples);

    fflush(stdout);
}


static void testAliasFunOrig(double normalizedFreq)
{
    // adjust the freq to even
    double freq = Analyzer::makeEvenPeriod(sampleRate * normalizedFreq, sampleRate, numSamples);
    printHeader("FunOrig", sampleRate * normalizedFreq, freq);

    VoltageControlledOscillatorOrig<16, 16> vco;
    vco.freq = float(sampleRate * normalizedFreq);
    vco.sampleTime = 1.0f / sampleRate;

    testAlias([&vco]() {
        const float deltaTime = 1.0f / sampleRate;
        vco.process(deltaTime, 0);
        return 15 * vco.saw();
        }, freq, numSamples);
}


static void testAliasFun(double normalizedFreq)
{
    // adjust the freq to even
    double freq = Analyzer::makeEvenPeriod(sampleRate * normalizedFreq, sampleRate, numSamples);
    printHeader("Fun Mine", sampleRate * normalizedFreq, freq);

    VoltageControlledOscillator<16, 16> vco;
    vco.freq = float(sampleRate * normalizedFreq);
    vco.sampleTime = 1.0f / sampleRate;

    vco.sawEnabled = true;
    vco.sinEnabled = false;
    vco.sqEnabled = false;
    vco.triEnabled = false;
    vco.init();

    testAlias([&vco]() {
        const float deltaTime = 1.0f / sampleRate;
        vco.process(deltaTime, 0);
        return 15 * vco.saw();
        }, freq, numSamples);
}


/*
First try:
desired freq = 844.180682, round 842.486572
test alias fundamental=842.486572,1684.973145,2527.459717
total sig = 3.239564 alias = 0.100040 ratiodb=-30.206276

desired freq = 1688.361365, round 1687.664795
test alias fundamental=1687.664795,3375.329590,5062.994385
total sig = 6.824180 alias = 0.158808 ratiodb=-32.663559

desired freq = 3376.722729, round 3375.329590
test alias fundamental=3375.329590,6750.659180,10125.988770
total sig = 3.512697 alias = 0.166856 ratiodb=-26.465975
Test passed. Press any key to continue...

---

EvenVCO freq = 844.180682, round 843.832397
frame size = 65536, expandThreshDb=0.000000
expand bins=0, adjustBins=1
adjust moved 6939
total sig = 0.015563 alias = 0.015805 ratiodb=0.133833

Raw Saw freq = 844.180682, round 843.832397
frame size = 65536, expandThreshDb=0.000000
expand bins=0, adjustBins=1
adjust moved 392
total sig = 0.166494 alias = 0.041957 ratiodb=-11.971910

Raw Saw freq = 1688.361365, round 1688.337708
frame size = 65536, expandThreshDb=0.000000
expand bins=0, adjustBins=1
adjust moved 0
total sig = 14.344683 alias = 1.279057 ratiodb=-20.996020

Raw Saw freq = 3376.722729, round 3376.675415
frame size = 65536, expandThreshDb=0.000000
expand bins=0, adjustBins=1
adjust moved 0
total sig = 10.917767 alias = 1.400212 ratiodb=-17.838803
Test passed. Press any key to continue...

----

EvenVCO freq = 844.180682, round 843.832397
frame size = 65536, expandThreshDb=0.000000
expand bins=1, adjustBins=1
adjust moved 6939
total sig = 0.015563 alias = 30.322415 ratiodb=65.793436

Raw Saw freq = 844.180682, round 843.832397
frame size = 65536, expandThreshDb=0.000000
expand bins=1, adjustBins=1
adjust moved 392
total sig = 0.166494 alias = 0.368204 ratiodb=6.893811

Raw Saw freq = 1688.361365, round 1688.337708
frame size = 65536, expandThreshDb=0.000000
expand bins=1, adjustBins=1
adjust moved 0
total sig = 14.344683 alias = 3.708226 ratiodb=-11.750495

Raw Saw freq = 3376.722729, round 3376.675415
frame size = 65536, expandThreshDb=0.000000
expand bins=1, adjustBins=1
adjust moved 0
total sig = 10.917767 alias = 3.809961 ratiodb=-9.144265
Test passed. Press any key to continue...




*/

void testVCOAlias()
{
    testPitchQuantize();


    for (int i = 2; i <= 8; i *= 2) {
        float f = 1.0f / (i * 6.53f);
      //  testEven(f);
       // testRawSaw<float>(f);
       // testAliasFunOrig(f);
        testAliasFun(f);
    }
}