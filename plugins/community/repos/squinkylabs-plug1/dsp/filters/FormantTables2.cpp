
#include <assert.h>
#include <cmath>

#include "AudioMath.h"
#include "FormantTables2.h"


static const float freqLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
   // model = 0(bass)
    {
        // F1 a
        //0=a,1=e,2=i, 3=o 4=u
        {600, 400, 250, 400, 350},
        // F2 
        {1040, 1620, 1750, 750, 600},
        // F3 
        {2250, 2400, 2600, 2400, 2400},
        // F4 
        {2450, 2800, 3050, 2600, 2675},
        //F5 
        {2750, 3100, 3340, 2900, 2950}
    },
    //1(tenor)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {650, 400, 290, 400, 350 },
        // F2
        {1080, 1700, 1870, 800, 600 },
        // F3
        {2650, 2600, 2800, 2600, 2700},
        // F4
        {2900, 3200, 3250, 2800, 2900},
        //F5
        {3250, 3850, 3540, 3000, 3300}
    },
    //2(countertenor)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {660, 440, 270, 430, 370},
        // F2
        {1120, 1800, 1850, 820, 630},
        // F3
        {2750, 2700, 2900, 2700, 2750},
        // F4
        {3000, 3000, 3350, 3000,  3000},
        //F5
        {3350, 3300, 3590, 3300, 3400}
    },
    //3(alto)
    {
        // F1     
        //0=a,1=e,2=i, 3=o 4=u
        {800, 400, 350, 450, 325},
        // F2
        {1150, 1600, 1700, 800, 700},
        // F3
        {2800, 2700, 2700, 2830, 2530},
        // F4
        {3500, 3300, 3700, 3500, 3500},
        //F5
        {4950, 4950, 4950, 4950, 4950}
    },
    // 4(soprano)
    {
        // F1
        //0=a,1=e,2=i, 3=o 4=u
        {800, 350, 270, 450,  325},
        // F2
        {1150, 2000,  2140, 800, 700},
        // F3
        {2900, 2800, 2950, 2830, 2700},
        // F4
        {3900, 3600, 3900, 3800, 3800},
        //F5
        {4950, 4950, 4950, 4950, 4950}
    }
};

static const float bwLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
    // model = 0(bass)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {60, 40, 60, 40, 40},
        // F2
        {70, 80, 90, 80, 80},
        // F3
        {110, 100, 100, 100, 100},
        // F4
        {120, 120, 120, 120, 120},
        //F5
        {130, 120, 120, 120, 120}
    },
    //1(tenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {80, 70, 40, 70, 40},
        // F2
        {90, 80, 90, 80, 60},
        // F3
        {120, 100, 100, 100, 100},
        // F4
        {130, 120, 120, 130, 120},
        // F5
        {140, 120, 120, 135, 120}
    },
    //2(countertenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {80, 70, 40, 40, 40},
        // F2
        {90, 80, 90, 80, 60},
        // F3
        {120, 100, 100, 100, 100},
        // F4
        {130, 120, 120, 120, 120},
        //F5
        {140, 120, 120, 120, 120}
    },
    //3(alto)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {80, 60, 50, 70, 50 },
        // F2
        {90, 80, 100, 80, 60},
        // F3
        {120, 120, 120, 100, 170},
        // F4
        {130, 150, 150, 130, 180},
        //F5
        {140, 200, 200, 135, 200}
    },
// 4(soprano)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {80, 60, 60, 40, 50},
        // F2
        {90, 100, 90, 80, 60},
        // F3
        {120, 120, 100, 100, 170},
        // F4
        {130, 150, 120, 120, 180},
        //F5
        {140, 200, 120, 120, 200}
    }
};

static const float gainLookup[FormantTables2::numModels][FormantTables2::numFormantBands][FormantTables2::numVowels] = {
    // model = 0(bass)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {0, 0, 0, 0, 0, },
        // F2
        {-7, -12, -30, -11, -20},
        // F3
        {-9, -9, -16, -21, -32},
        // F4
        {-12, -12, -22, -20, -28},
        //F5
        {-18, -18, -28, -40, -36}
    },
    //1(tenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {0, 0, 0, 0, 0},
        // F2
        {-6, -14, -15, -10, -20},
        // F3
        {-7, -12, -18, -12, -17},
        // F4
        {-8, -14, -20, -12, -14},
        //F5
        {-22, -20, -30, -26, -26}
    },
    //2(countertenor)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {0, 0, 0, 0, 0},
        // F2
        {-6, -14, -24, -10, -20},
        // F3
        {-23, -18, -24, -26, -23},
        // F4
        {-24, -20, -36, -22, -30},
        //F5
        {-38, -20, -36, -34, -34}
    },
    //3(alto)
    {
        // F1      0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {0, 0, 0, 0, 0},
        // F2
        {-4, -24, -30, -9, -12},
        // F3
        {-20, -30, -30, -16, -30},
        // F4
        {-36, -35, -36, -28, -40},
        //F5
        {-60, -60, -60, -55, -64}
    },
    // 4(soprano)
    {
        // F1     0 = a, 1 = e, 2 = i, 3 = o 4 = u
        {0, 0, 0, 0, 0},
        // F2
        {-6, -20, -12, -11, -16},
        // F3
        {-32, -15, -26, -22, -35},
        // F4
        {-20, -40, -26, -22, -40},
        //F5
        {-50, -56, -44, -50, -60}
    }
};

FormantTables2::FormantTables2()
{

    for (int model = 0; model < numModels; ++model) {
        for (int formantBand = 0; formantBand < numFormantBands; ++formantBand) {

            LookupTableParams<float>& fparams = freqInterpolators[model][formantBand];
            const float  *freqValues = freqLookup[model][formantBand];

            // It would be more efficient to init the tables with logs, but this
            // on the fly conversion is fine.
            float temp[numVowels];
            for (int vowel = 0; vowel < numVowels; ++vowel) {
                temp[vowel] = std::log2(freqValues[vowel]);
            }
            LookupTable<float>::initDiscrete(fparams, numVowels, temp);

            // Init Wb lookups with normalized bw ( f2-f1 / fc)
            LookupTableParams<float>& bwparams = bwInterpolators[model][formantBand];
            const float *bwValues = bwLookup[model][formantBand];
            for (int vowel = 0; vowel < numVowels; ++vowel) {
                temp[vowel] = bwValues[vowel] / freqValues[vowel];
            }

            LookupTable<float>::initDiscrete(bwparams, numVowels, temp);

            LookupTableParams<float>& gparams = gainInterpolators[model][formantBand];
            const float* gainValues = gainLookup[model][formantBand];
            for (int vowel = 0; vowel < numVowels; ++vowel) {
              //  temp[vowel] = (float) AudioMath::gainFromDb(gainValues[vowel]);
                // let's leave these as db
                temp[vowel] = gainValues[vowel];
            }
            LookupTable<float>::initDiscrete(gparams, numVowels, temp);
        }
    }
}

float FormantTables2::getLogFrequency(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = freqInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}

float FormantTables2::getNormalizedBandwidth(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = bwInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}
float FormantTables2::getGain(int model, int formantBand, float vowel)
{
    assert(model >= 0 && model <= numModels);
    assert(formantBand >= 0 && formantBand <= numFormantBands);
    assert(vowel >= 0 && vowel < numVowels);

    LookupTableParams<float>& params = gainInterpolators[model][formantBand];
    return LookupTable<float>::lookup(params, vowel);
}