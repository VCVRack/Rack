#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#include "ObjectCache.h"

class IIRUpsampler
{
public:
    void process(float * outputBuffer, float input)
    {
        input *= oversample;
        for (int i = 0; i < oversample; ++i) {
            outputBuffer[i] = BiquadFilter<float>::run(input, state, *params);
            input = 0;      // just filter a delta - don't average the whole signal
        }
    }
    /**
    * cutoff is normalized freq (.5 = nyquist).
    * typically cutoff will be <  (.5 / FACTOR),
    * if not, the filters wouldn't work.
    */
#if 0
    void setCutoff(float cutoff)
    {
        assert(cutoff > 0 && cutoff < .5f);
       // ButterworthFilterDesigner<float>::designSixPoleLowpass(params, cutoff);
        params = ObjectCache<float>::get6PLPParams(cutoff);
    }
#endif

    /**
    * Will set the oversample factor, and set the filter cutoff.
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