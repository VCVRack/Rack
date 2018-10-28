
#include "AudioMath.h"
#include "FFT.h"
#include "FFTData.h"

#include <assert.h>
#include "kiss_fft.h"
#include "kiss_fftr.h"

#include "AudioMath.h"


bool FFT::forward(FFTDataCpx* out, const FFTDataReal& in)
{
    if (out->buffer.size() != in.buffer.size()) {
        return false;
    }

    // step 1: create the cfg, if needed
    if (in.kiss_cfg == 0) {
        bool inverse_fft = false;
        kiss_fftr_cfg newCfg=  kiss_fftr_alloc((int)in.buffer.size(),
            inverse_fft,
           nullptr, nullptr);

        assert (newCfg);
        if (!newCfg) {
            return false;
        }

        // now save off in our typeless pointer.
        assert(sizeof(newCfg) == sizeof(in.kiss_cfg));
        in.kiss_cfg = newCfg;
    }

    // step 2: do the fft
    kiss_fftr_cfg theCfg = reinterpret_cast<kiss_fftr_cfg>(in.kiss_cfg);

    // TODO: need a test that this assumption is correct (that we kiss_fft_cpx == std::complex.
    kiss_fft_cpx * outBuffer = reinterpret_cast<kiss_fft_cpx *>(out->buffer.data());
    kiss_fftr(theCfg, in.buffer.data(), outBuffer);

    // step 4: scale
    const float scale = float(1.0 / in.buffer.size());
    for (size_t i = 0; i < in.buffer.size(); ++i) {
        out->buffer[i] *= scale;
    }

    return true;
}

bool FFT::inverse(FFTDataReal* out, const FFTDataCpx& in)
{
    if (out->buffer.size() != in.buffer.size()) {
        return false;
    }

    // step 1: create the cfg, if needed
    if (in.kiss_cfg == 0) {
        bool inverse_fft = true;
        kiss_fftr_cfg newCfg = kiss_fftr_alloc((int) in.buffer.size(),
            inverse_fft,
            nullptr, nullptr);

        assert(newCfg);
        if (!newCfg) {
            return false;
        }

        // now save off in our typeless pointer.
        assert(sizeof(newCfg) == sizeof(in.kiss_cfg));
        in.kiss_cfg = newCfg;
    }

    // step 2: do the fft
    kiss_fftr_cfg theCfg = reinterpret_cast<kiss_fftr_cfg>(in.kiss_cfg);

    // TODO: need a test that this assumption is correct (that we kiss_fft_cpx == std::complex.
    const kiss_fft_cpx * inBuffer = reinterpret_cast<const kiss_fft_cpx *>(in.buffer.data());

    kiss_fftri(theCfg, inBuffer, out->buffer.data());
    return true;
}

int FFT::freqToBin(double freq, double sampleRate, int numBins)
{
    assert(freq <= (sampleRate / 2));
    // bin(numBins) <> sr / 2;
    return (int)((freq / sampleRate)*(numBins));
}

double FFT::bin2Freq(int bin, double sampleRate, int numBins)
{
    return  sampleRate * double(bin) / double(numBins);
}

static float randomPhase()
{
    float phase = (float) rand();
    phase = phase / (float) RAND_MAX;   // 0..1
    phase = (float) (phase * (2 * AudioMath::Pi));
    return phase;
}

static void makeNegSlope(FFTDataCpx* output, const ColoredNoiseSpec& spec)
{
    const int numBins = int(output->size());
    const float lowFreqCorner = 40;

    // find bin for 40 Hz
    const int bin40 = FFT::freqToBin(lowFreqCorner, spec.sampleRate, numBins);
 
    // fill bottom bins with 1.0 mag
    for (int i = 0; i <= bin40; ++i) {
        output->set(i, std::polar(1.f, randomPhase()));
    }

    // now go to the end and at slope
    static float k = -spec.slope * log2(lowFreqCorner);
    for (int i = bin40 + 1; i < numBins; ++i) {
        if (i < numBins / 2) {
            const double f = FFT::bin2Freq(i, spec.sampleRate, numBins);
            const double gainDb = std::log2(f) * spec.slope + k;
            const float gain = float(AudioMath::gainFromDb(gainDb));
            output->set(i, std::polar(gain, randomPhase()));
        } else {
            output->set(i, cpx(0, 0));
        }
    }
    output->set(0, 0);          // make sure dc bin zero
}

static void makePosSlope(FFTDataCpx* output, const ColoredNoiseSpec& spec)
{
    const int numBins = int(output->size());

    // find bin for high corner
    const int binHigh = FFT::freqToBin(spec.highFreqCorner, spec.sampleRate, numBins);

    // now go to the end and at slope
    float gainMax = 1;              // even if nothing in the bins (ut) needs something in there.
    static float k = -spec.slope * log2(spec.highFreqCorner);
    for (int i = binHigh - 1; i > 0; --i) {
        if (i < numBins / 2) {
            const double f = FFT::bin2Freq(i, spec.sampleRate, numBins);
            const double gainDb = std::log2(f) * spec.slope + k;
            const float gain = float(AudioMath::gainFromDb(gainDb));
            gainMax = std::max(gain, gainMax);
            output->set(i, std::polar(gain, randomPhase()));
        } else {
            output->set(i, cpx(0, 0));
        }
    }

    // fill top bins with mag mag
    for (int i = numBins - 1; i >= binHigh; --i) {
        if (i < numBins / 2) {
            output->set(i, std::polar(gainMax, randomPhase()));
        } else {
            output->set(i, cpx(0.0));
        }
    }

    output->set(0, 0);          // make sure dc bin zero
}

void FFT::makeNoiseSpectrum(FFTDataCpx* output, const ColoredNoiseSpec& spec)
{
    // for now, zero all first.
    const int frameSize = (int) output->size();
    for (int i = 0; i < frameSize; ++i) {
        cpx x(0,0);
        output->set(i, x);
    }
    if (spec.slope < 0) {
        makeNegSlope(output, spec);
    } else {
        makePosSlope(output, spec);
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

void FFT::normalize(FFTDataReal* data, float maxValue)
{
    assert(maxValue > 0);
    const float peak = getPeak(*data);
    const float correction = maxValue / peak;
    for (int i = 0; i < data->size(); ++i) {
        float x = data->get(i);
        x *= correction;
        data->set(i, x);
    }
}