#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#include "ObjectCache.h"

/**
 * A traditional decimator, using IIR filters for interpolation.
 * Takes a signal at a higher sample rate, and reduces it to a lower
 * sample rate without adding (much) aliasing.
 */
class IIRDecimator
{
public:
    /**
     * Will set the oversample factor, and set the filter cutoff.
     * Normalized cutoff is fixed at 1 / 4 * oversample, so most of the
     * top octave will be filtered out.
     */
    void setup(int oversampleFactor)
    {
        if (oversampleFactor != oversample) {
            oversample = oversampleFactor;

            // Set out IIR filter to be a six pole butterworth lowpass and the magic frequency.
            params = ObjectCache<float>::get6PLPParams(1.f / (4.0f * oversample));
        }
    }

    /**
     * Down-sample a buffer of data.
     * input is just an array of floats, the size is our oversampling factor.
     *
     * return value is a single sample
     */
    float process(const float * input)
    {
        float x = 0;
        for (int i = 0; i < oversample; ++i) {
            // The key here is to filter out all the frequencies that will
            // be higher than the destination Nyquist frequency.
            x = BiquadFilter<float>::run(input[i], state, *params);
        }
        // Note that we return just the last sample, and ignore the others.
        // Decimator is supposed to only keep one out of 'n' samples. We could 
        // average them all, but that would just apply a little undesired high
        // frequency roll-off.
        return x;
    }

private:
    /**
     * This is the oversampling factor. For example,
     * for 4x oversample, 'oversample' will be set to 4.
     */
    int oversample = -1;

    /**
     * "standard" Squinky Labs Biquad filter data.
     */
    std::shared_ptr<BiquadParams<float, 3>> params;
    BiquadState<float, 3> state;
};