#pragma once

#include <string.h>
#include "dsp/ringbuffer.hpp"

#define RS_BUFFER_SIZE 512
#define UPSAMPLE_COMPENSATION 1.3


namespace dsp {

/**
 * @brief Base class for all signal processors
 */
struct DSPEffect {

public:

    float sr = 44100.0;


    explicit DSPEffect(float sr) : sr(sr) {
        init();
    }


    float getSamplerate() const {
        return sr;
    }


    virtual void setSamplerate(float sr) {
        DSPEffect::sr = sr;
        invalidate();
    }


    virtual void init() {};


    /**
     * @brief Method for mark parameters as invalidate to trigger recalculation
     */
    virtual void invalidate() {};


    /**
     * @brief Process one step and return the computed sample
     * @return
     */
    virtual void process() {};
};


/** The normalized sinc function. https://en.wikipedia.org/wiki/Sinc_function */
inline double sinc(double x) {
    if (x == 0.)
        return 1.;
    x *= M_PI;
    return sin(x) / x;
}


/** Computes the impulse response of a boxcar lowpass filter */
inline void boxcarLowpassIR(double *out, int len, double cutoff = 0.5) {
    for (int i = 0; i < len; i++) {
        double t = i - (len - 1) / 2.;
        out[i] = 2 * cutoff * sinc(2 * cutoff * t);
    }
}


inline void blackmanHarrisWindow(double *x, int len) {
    // Constants from https://en.wikipedia.org/wiki/Window_function#Blackman%E2%80%93Harris_window
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;
    double factor = 2 * M_PI / (len - 1);
    for (int i = 0; i < len; i++) {
        x[i] *= +a0
                - a1 * cos(1 * factor * i)
                + a2 * cos(2 * factor * i)
                - a3 * cos(3 * factor * i);
    }
}


struct Decimator {
    double inBuffer[RS_BUFFER_SIZE];
    double kernel[RS_BUFFER_SIZE];
    int inIndex;
    int oversample, quality;
    double cutoff = 0.65;


    Decimator(int oversample, int quality) {
        Decimator::oversample = oversample;
        Decimator::quality = quality;

        boxcarLowpassIR(kernel, oversample * quality, cutoff * 0.5 / oversample);
        blackmanHarrisWindow(kernel, oversample * quality);
        reset();
    }


    void reset() {
        inIndex = 0;
        memset(inBuffer, 0, sizeof(inBuffer));
    }


    /** `in` must be length OVERSAMPLE */
    float process(double *in) {
        // Copy input to buffer
        memcpy(&inBuffer[inIndex], in, oversample * sizeof(double));
        // Advance index
        inIndex += oversample;
        inIndex %= oversample * quality;
        // Perform naive convolution
        double out = 0.;
        for (int i = 0; i < oversample * quality; i++) {
            int index = inIndex - 1 - i;
            index = (index + oversample * quality) % (oversample * quality);
            out += kernel[i] * inBuffer[index];
        }
        return out;
    }
};


struct Upsampler {
    double inBuffer[RS_BUFFER_SIZE];
    double kernel[RS_BUFFER_SIZE];
    int inIndex;
    int oversample, quality;
    double cutoff = 0.65;


    Upsampler(int oversample, int quality) {
        Upsampler::oversample = oversample;
        Upsampler::quality = quality;

        boxcarLowpassIR(kernel, oversample * quality, cutoff * 0.5 / oversample);
        blackmanHarrisWindow(kernel, oversample * quality);
        reset();
    }


    void reset() {
        inIndex = 0;
        memset(inBuffer, 0, sizeof(inBuffer));
    }


    /** `out` must be length OVERSAMPLE */
    void process(double in, double *out) {
        // Zero-stuff input buffer
        inBuffer[inIndex] = oversample * in;
        // Advance index
        inIndex++;
        inIndex %= quality;
        // Naively convolve each sample
        // TODO replace with polyphase lpf hierarchy
        for (int i = 0; i < oversample; i++) {
            float y = 0.0;
            for (int j = 0; j < quality; j++) {
                int index = inIndex - 1 - j;
                index = (index + quality) % quality;
                int kernelIndex = oversample * j + i;
                y += kernel[kernelIndex] * inBuffer[index];
            }
            out[i] = y;
        }
    }
};


/**
 * @brief NEW oversampling class
 */
template<int CHANNELS>
struct Resampler {
    struct Vector {
        double y0, y1;
    };

    Vector y[CHANNELS] = {};
    double up[CHANNELS][RS_BUFFER_SIZE] = {};
    double data[CHANNELS][RS_BUFFER_SIZE] = {};

    Decimator *decimator[CHANNELS];
    Upsampler *interpolator[CHANNELS];

    int oversample;


    /**
     * @brief Constructor
     * @param factor Oversampling factor
     */
    Resampler(int oversample, int quality = 4) {
        Resampler::oversample = oversample;

        for (int i = 0; i < CHANNELS; i++) {
            decimator[i] = new Decimator(oversample, quality);
            interpolator[i] = new Upsampler(oversample, quality);
        }
    }


    int getFactor() {
        return oversample;
    }


    /**
     * @brief Return linear interpolated position
     * @param point Point in oversampled data
     * @return
     */
    double interpolate(int channel, int point) {
        return y[channel].y0 + (point / getFactor()) * (y[channel].y1 - y[channel].y0);
    }


    /**
     * @brief Create up-sampled data out of two basic values
     */
    void doUpsample(int channel, double in) {
        interpolator[channel]->process(in * UPSAMPLE_COMPENSATION, up[channel]);
        /*  y[channel].y0 = y[channel].y1;
          y[channel].y1 = in;

          for (int i = 0; i < getFactor(); i++) {
              up[channel][i] = interpolate(channel, i + 1);
          }*/
    }


    /**
     * @brief Downsampled data from a given channel
     * @param channel Channel to proccess
     * @return Downsampled point
     */
    double getDownsampled(int channel) {
        return decimator[channel]->process(data[channel]);
    }


    /**
     * @brief Upsampled data from a given channel
     * @param channel Channel to retrieve
     * @return Pointer to the upsampled data
     */
    double *getUpsampled(int channel) {
        return up[channel];
    }
};

}