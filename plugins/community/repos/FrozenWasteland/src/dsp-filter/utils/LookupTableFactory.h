#pragma once

#include "LookupTable.h"

// TODO: this class should not be templatized. the functions should
template<typename T>
class LookupTableFactory
{
public:
    static void makeBipolarAudioTaper(LookupTableParams<T>& params);
    static double audioTaperKnee()
    {
        return -24;
    }
    /**
    * Factory methods for exp base 2
    * domain = 0..10
    * range = 20..20k (for now). but should be .001 to 1.0?
    */
    static void makeExp2(LookupTableParams<T>& params);
    static double expYMin()
    {
        return  4;
    }
    static double expYMax()
    {
        return  40000;
    }
    static double expXMin()
    {
        return  std::log2(expYMin());
    }
    static double expXMax()
    {
        return  std::log2(expYMax());
    }
};


template<typename T>
inline void LookupTableFactory<T>::makeExp2(LookupTableParams<T>& params)
{
    // 128 not enough for one cent
    const int bins = 256;
    const T xMin = (T) std::log2(expYMin());
    const T xMax = (T) std::log2(expYMax());
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return std::pow(2, x);
        });
}

template<typename T>
inline void LookupTableFactory<T>::makeBipolarAudioTaper(LookupTableParams<T>& params)
{
    const int bins = 32;
    std::function<double(double)> audioTaper = AudioMath::makeFunc_AudioTaper(audioTaperKnee());
    const T xMin = -1;
    const T xMax = 1;
    LookupTable<T>::init(params, bins, xMin, xMax, [audioTaper](double x) {
        return (x >= 0) ? audioTaper(x) : -audioTaper(-x);
        });

}