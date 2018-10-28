#pragma once

#include "BiquadParams.h"
#include "ButterworthFilterDesigner.h"
#include "LookupTable.h"

/**
* Interpolating lookup for filter parameters
*/
class ButterworthLookup4PHP
{
public:
    ButterworthLookup4PHP();
    void get(BiquadParams<float, 2>& params, float normalizedCutoff);
private:
    static const int numTables = 10;
    LookupTableParams<float> tables[numTables];    // five params per two biquads
};

inline ButterworthLookup4PHP::ButterworthLookup4PHP()
{
    const int numBins = 256;
    for (int index = 0; index < numTables; ++index) {
        LookupTable<float>::init(tables[index], numBins, 100.0f / 44100.0f, 2000 / 44100.0f, [index](double x) {
            // first design a filter at x hz
            BiquadParams<float, 2> params;
            ButterworthFilterDesigner<float>::designFourPoleHighpass(params, float(x));
            // then save off tap 0;
            return params.getAtIndex(index);
            });
    }
}

inline void ButterworthLookup4PHP::get(BiquadParams<float, 2>& params, float normalizedCutoff)
{
    for (int i = 0; i < numTables; ++i) {
       // const int stage = i < numTables / 2;
        float p = LookupTable<float>::lookup(tables[i], normalizedCutoff, true);
        params.setAtIndex(p, i);
    }
}