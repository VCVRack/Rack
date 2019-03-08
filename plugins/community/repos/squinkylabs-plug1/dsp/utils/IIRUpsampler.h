#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#include "ObjectCache.h"

/**
 * Inverse of the IIRDecimator.
 * takes a single sample at a lower sample rate, and converts it
 * to a buffer of data at the higher sample rate
 */
class IIRUpsampler
{
public:

    /**
    * Will set the oversample factor, and set the filter cutoff.
    * Normalized cutoff is fixed at 1 / 4 * oversample, so that
    * much of the upper octave will be filtered out before upsampling.
    */
    void setup(int oversampleFactor)
    {
        oversample = oversampleFactor;
        params = ObjectCache<float>::get6PLPParams(1.f / (4.0f * oversample));
    }

    /**
     * processes one sample of input. Output is a buffer of data at the
     * higher sample rate. Buffer size is just the oversample amount.
     *
     * We are going to "zero" pack our data. For example, if input is [a, b],
     * and oversample is 4, then we would first zero pack like so:
     * [a, 0, 0, 0, b, 0, 0, 0]. Then we filter the results. Again, this is correct -
     * repeating the data like [a, a, a, a, b, b, b, b] would give a slight roll-off that
     * we don't want.
     */
    void process(float * outputBuffer, float input)
    {
        // The zero packing reduced the overall volume. To preserve the volume,
        // multiply be the reduction amount, which is oversample.
        input *= oversample;

        for (int i = 0; i < oversample; ++i) {
            outputBuffer[i] = BiquadFilter<float>::run(input, state, *params);
            input = 0;      // just filter a delta - don't average the whole signal (i.e. zero pack)
        }
    }

private:
    int oversample = 16;

    std::shared_ptr<BiquadParams<float, 3>> params;
    BiquadState<float, 3> state;
};