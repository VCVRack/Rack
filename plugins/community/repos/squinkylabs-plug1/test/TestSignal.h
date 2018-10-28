#pragma once

#include <cmath>
#include <vector>

/**
 * Utilities for generating and analyzing test signals.
 *
 * All test signals are +-1volt unless otherwise specified
 */

template <typename T>
class TestSignal
{
public:
    TestSignal() = delete;       // we are only static
    // freq is normalized f / sr
    static void generateSin(T * output, int numSamples, T freq);
    static double getRMS(const T * signal, int numSamples);

    /**
     * Measure the gain of a processing lambda at a specificed frequency
     */
    static T measureGain(T freq, std::function<T(T)> func);

    /**
     * Measure the RMS output of a generator lambda
     */
    static double measureOutput(int samples, std::function<T()> func);
};


template <typename T>
inline void TestSignal<T>::generateSin(T * output, int numSamples, T freq)
{
    assert(freq < .5);
    assert(freq > 0);

    double phase = 0;
    for (int i = 0; i < numSamples; ++i) {
        const double phi = phase * 2 * AudioMath::Pi;

        output[i] = T(std::sin(phi));
        phase += freq;
        // should we normalize the phase here, instead of letting it grow?
    }
}

template <typename T>
inline double TestSignal<T>::getRMS(const T * signal, int numSamples)
{
    assert(numSamples > 0);
    double sumSq = 0;
    for (int i = 0; i < numSamples; ++i) {
        //printf("getRMS, i=%d, samp=%d\n")
        sumSq += double(signal[i]) * signal[i];
    }
    return std::sqrt(sumSq / numSamples);
}

template <typename T>
inline T TestSignal<T>::measureGain(T freq, std::function<T(T)> func)
{
    const int size = 60000;
    T input[size];
    T output[size];

    //generateSin(T * output, int numSamples, T freq);
    TestSignal<T>::generateSin(input, size, freq);
    for (int i = 0; i < size; ++i) {
        output[i] = func(input[i]);
    }
    T vo = T(TestSignal<T>::getRMS(output, size));
    T vi = T(TestSignal<T>::getRMS(input, size));
    return vo / vi;
}

template <typename T>
inline double TestSignal<T>::measureOutput(int numSamples, std::function<T()> func)
{
    //std::unique_ptr<T> buffer(new T[numSamples]);
    std::vector<T> buffer(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = func();
    }

    return getRMS(buffer.data(), numSamples);

}