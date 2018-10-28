#pragma once

#include <set>
#include <vector>
#include <functional>

#include "FFTData.h"
#include "FFT.h"


class Analyzer
{
public:
    Analyzer() = delete;

    class FPoint
    {
    public:
        FPoint(float f, float g) : freq(f), gainDb(g)
        {
        }
        float freq;
        float gainDb;
    };

    static std::vector<FPoint> getFeatures(const FFTDataCpx&, float sensitivityDb, float sampleRate, float minDb);
    static std::vector<FPoint> getPeaks(const FFTDataCpx&, float sampleRate, float minDb);
    static void getAndPrintFeatures(const FFTDataCpx&, float sensitivityDb, float sampleRate, float minDb);
    static void getAndPrintPeaks(const FFTDataCpx&, float sampleRate, float minDb);
    static void getAndPrintFreqOfInterest(const FFTDataCpx&, float sampleRate, const std::vector<double>& freqOfInterest);

    static int getMax(const FFTDataCpx&);
    static int getMaxExcluding(const FFTDataCpx&, std::set<int> exclusions);
    static int getMaxExcluding(const FFTDataCpx&, int exclusion);

    /**
     * 0 = low freq bin #
     * 1 = peak bin #
     * 2 = high bin#
     * dbAtten (typically -3
     */
    static std::tuple<int, int, int> getMaxAndShoulders(const FFTDataCpx&, float dbAtten);

    /**
    * 0 = low freq
    * 1 = peak freq
    * 2 = high freq
    * dbAtten (typically -3
    */
    static std::tuple<double, double, double> getMaxAndShouldersFreq(const FFTDataCpx&, float dbAtten, float sampleRate);

    /**
     * Calculates the frequency response of func
     * by calling it with a known test signal.
     */
    static void getFreqResponse(FFTDataCpx& out, std::function<float(float)> func);

    /**
     * Calculates the spectrum of func
     * by calling it can capturing its output
     */
    static void getSpectrum(FFTDataCpx& out, bool useWindow, std::function<float()> func);

    static float getSlope(const FFTDataCpx& response, float fTest, float sampleRate);

    static void generateSweep(float sampleRate, float* out, int numSamples, float minFreq, float maxFreq);

    /**
     * Adjusts desiredFreq to a frequency that is close, but is an exact division of
     * numSamples.
     */
    static double makeEvenPeriod(double desiredFreq, double sampleRate, int numSamples);

    static double hamming(int iSample, int totalSamples);

    /**
     * Assert that there is a single frequency in spectrum, and that it is close to
     * expectedFreq.
     *
     * In other words, check that the signal was a reasonably pure sin.
     */
    static void assertSingleFreq(const FFTDataCpx& spectrum, float expectedFreq, float sampleRate);
};
