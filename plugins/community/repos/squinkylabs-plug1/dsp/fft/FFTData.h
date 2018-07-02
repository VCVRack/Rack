#pragma once

#include <complex>
#include <vector>


class FFT;

/**
 * Our wrapper api uses std::complex, so we don't need to expose kiss_fft_cpx
 * outside. Our implementation assumes the two are equivalent, and that a
 * reinterpret_cast can bridge them.
 */
using cpx = std::complex<float>;

class FFTDataCpx
{
public:
    friend FFT;
    FFTDataCpx(int numBins);
    ~FFTDataCpx();
    cpx get(int bin) const;
    void set(int bin, cpx value);

    int size() const
    {
        return (int) buffer.size();
    }
    static int _count;
private:
    std::vector<cpx> buffer;

    /**
    * we store this without type so that clients don't need
    * to pull in the kiss_fft headers. It's mutable so it can
    * be lazy created by FFT functions.
    * Note that the cfg has a "direction" baked into it. For
    * now we assume that all FFT with complex input will be inverse FFTs.
    */
    mutable void * kiss_cfg = 0;
};

/**
 * Holds an fft frame of real data.
 */
class FFTDataReal
{
public:
    friend FFT;
    FFTDataReal(int numBins);
    ~FFTDataReal();
    float get(int numBin) const;
    void set(int numBin, float value);
    int size() const
    {
        return (int) buffer.size();
    }
    static int _count;
private:
    std::vector<float> buffer;

    /**
     * we store this without type so that clients don't need
     * to pull in the kiss_fft headers. It's mutable so it can
     * be lazy created by FFT functions.
     * Note that the cfg has a "direction" baked into it. For
     * now we assume that all FFT with real input will be forward FFTs.
     */
    mutable void * kiss_cfg = 0;
};