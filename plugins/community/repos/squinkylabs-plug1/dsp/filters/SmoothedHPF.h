#pragma once


#include "BiquadState.h"
#include "BiquadFilter.h"
#include "ButterworthLookup.h"
#include "MultiLag.h"

#include <assert.h>
//#define _LOG

/**
 * SmoothedHPF
 *
 * A four-pole butterworth high-pass with some unusual characteristics.
 *      Cutoff set via fast lookup table, but not super precise.
 *      Coefficient smoothing to avoid pops when Fc changes.
 */
class SmoothedHPF
{
public:
    SmoothedHPF();

    float run(float);
    void setCutoff(float);
private:
    ButterworthLookup4PHP filterLookup;
    MultiLPF<12> smoother;

    BiquadState<float, 2> filterState;
    BiquadParams<float, 2> currentParams;       // after lagging, the ones we filter with
    BiquadParams<float, 2> finalParams;          // input to the lag - the params at infinity
};

inline SmoothedHPF::SmoothedHPF()
{
    // .00001 is crazy (
    // .0001 sounds decent
    // .001 pops (44 hz
    smoother.setCutoff(.0005f);
}

inline float SmoothedHPF::run(float input)
{
    smoother.step(finalParams.taps());      // problem here is that there are only 10 taps.
    for (int i = 0; i < 10; ++i) {
        float x = smoother.get(i);
        currentParams.setAtIndex(x, i);
#ifdef _LOG
        printf("%.2f ", x);
#endif
   }
#ifdef _LOG
   printf("\n"); fflush(stdout);
#endif
   return BiquadFilter<float>::run(input, filterState, currentParams);
}

inline void  SmoothedHPF::setCutoff(float normalizedCutoff)
{
#ifdef _LOG
    printf("set cutoff %.2f ", normalizedCutoff);
#endif
    filterLookup.get(finalParams, normalizedCutoff);
#ifdef _LOG
    for (int i = 0; i < 10; ++i) {
        printf("%.2f ", finalParams.getAtIndex(i));
    }
    printf("\n"); fflush(stdout);
#endif
}