#pragma once
#include "LookupTable.h"

class FormantTables2
{
public:
    static const int numVowels = 5;
    static const int numModels = 5;
    static const int numFormantBands = 5;
    /**
    * Interpolates the frequency using lookups
    * @param model =  0(bass)  1(tenor) 2(countertenor) 3(alto)  4(soprano)
    * @param index = 0..4 (formant F1..F5)
    * @param vowel is the continuous index into the per/vowel lookup tables (0..4)
    *  0 = a, 1 = e, 2=i, 3 = o 4 = u
    */
    float getLogFrequency(int model, int index, float vowel);
    float getNormalizedBandwidth(int model, int index, float vowel);
    float getGain(int model, int index, float vowel);

    FormantTables2();
    FormantTables2(const FormantTables2&) = delete;
    const FormantTables2& operator==(const FormantTables2&) = delete;
private:

    LookupTableParams<float> freqInterpolators[numModels][numFormantBands];
    LookupTableParams<float> bwInterpolators[numModels][numFormantBands];
    LookupTableParams<float> gainInterpolators[numModels][numFormantBands];
};
