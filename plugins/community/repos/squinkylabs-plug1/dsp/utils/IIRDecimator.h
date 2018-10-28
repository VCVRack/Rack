#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"

/**
 * A traditional decimator, using IIR filters for interpolation
 *
 * template parameter OVERSAMPLE is the 
 * decimation rate.
 */
class IIRDecimator
{
public:
    float process(const float * input)
    {
        float x = 0;
        for (int i = 0; i < oversample; ++i) {
            x = BiquadFilter<float>::run(input[i], state, *params);
        }
        return x;
    }

    /**
     * cutoff is normalized freq (.5 = nyquist).
     * typically cutoff will be <  (.5 / OVERSAMPLE), 
     * if not, the filters wouldn't work.
     */
#if 0
    void setCutoff(float cutoff)
    {
        assert(cutoff > 0 && cutoff < .5f);
        params = ObjectCache<float>::get6PLPParams(cutoff);
    }
#endif
    /**
     * will set the oversample factor, and set the filter cutoff.
     * NOrmalized cutoff is fixed at 1 / 4 * oversample, for a one
     * octave passband
     */
    void setup(int oversampleFactor)
    {
        assert(oversampleFactor == 4 || oversampleFactor == 16);
        oversample = oversampleFactor;
        params = ObjectCache<float>::get6PLPParams(1.f / (4.0f * oversample));
    }
private:
    std::shared_ptr<BiquadParams<float, 3>> params;
    BiquadState<float, 3> state;
    int oversample = 16;
};