
#include <assert.h>

#include "AudioMath.h"
#include "ButterworthFilterDesigner.h"
#include "LookupTableFactory.h"
#include "ObjectCache.h"

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getBipolarAudioTaper()
{
    std::shared_ptr< LookupTableParams<T>> ret = bipolarAudioTaper.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTableFactory<T>::makeBipolarAudioTaper(*ret);
        bipolarAudioTaper = ret;
    }
    return ret;
}

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getAudioTaper()
{

    std::shared_ptr< LookupTableParams<T>> ret = audioTaper.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTableFactory<T>::makeAudioTaper(*ret);
        audioTaper = ret;
    }
    return ret;

    return nullptr;
}

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getSinLookup()
{
    std::shared_ptr< LookupTableParams<T>> ret = sinLookupTable.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        std::function<double(double)> f = AudioMath::makeFunc_Sin();
        // Used to use 4096, but 512 gives about 92db  snr, so let's save memory
        LookupTable<T>::init(*ret, 512, 0, 1, f);
        sinLookupTable = ret;
    }
    return ret;
}

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getExp2()
{
    std::shared_ptr< LookupTableParams<T>> ret = exp2.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTableFactory<T>::makeExp2(*ret);
        exp2 = ret;
    }
    return ret;
}

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getExp2ExtendedLow()
{
    std::shared_ptr< LookupTableParams<T>> ret = exp2ExLow.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTableFactory<T>::makeExp2ExLow(*ret);
        exp2ExLow = ret;
    }
    return ret;
}

template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getExp2ExtendedHigh()
{
    std::shared_ptr< LookupTableParams<T>> ret = exp2ExHigh.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTableFactory<T>::makeExp2ExHigh(*ret);
        exp2ExHigh = ret;
    }
    return ret;
}



template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getDb2Gain()
{
    std::shared_ptr< LookupTableParams<T>> ret = db2Gain.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTable<T>::init(*ret, 32, -80, 20, [](double x) {
            return AudioMath::gainFromDb(x);
            });
        db2Gain = ret;
    }
    return ret;
}


template <typename T>
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getTanh5()
{
    std::shared_ptr< LookupTableParams<T>> ret = tanh5.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        LookupTable<T>::init(*ret, 256, -5, 5, [](double x) {
            return std::tanh(x);
            });
        tanh5 = ret;
    }
    return ret;
}

/**
 * Lambda capture two smart pointers to lookup table params,
 * so lifetime of the lambda control their reft.
 */
template <typename T>
std::function<T(T)> ObjectCache<T>::getExp2Ex()
{
    std::shared_ptr < LookupTableParams<T>> low = getExp2ExtendedLow();
    std::shared_ptr < LookupTableParams<T>> high = getExp2ExtendedHigh();
    const T xDivide = (T) LookupTableFactory<T>::exp2ExHighXMin();
    return [low, high, xDivide](T x) {
        auto params = (x < xDivide) ? low : high;
        return LookupTable<T>::lookup(*params, x, true);
    };
}

template <typename T>
std::shared_ptr<BiquadParams<float, 3>> ObjectCache<T>::get6PLPParams(float normalizedFc)
{
    const int div = (int) std::round(1.0 / normalizedFc);
    if (div == 64) {
        std::shared_ptr < BiquadParams<float, 3>> ret = lowpass64.lock();
        if (!ret) {
            ret = std::make_shared<BiquadParams<float, 3>>();
            ButterworthFilterDesigner<float>::designSixPoleLowpass(*ret, normalizedFc);
            lowpass64 = ret;
        }
        return ret;
    } else if (div == 16) {
        std::shared_ptr < BiquadParams<float, 3>> ret = lowpass16.lock();
        if (!ret) {
            ret = std::make_shared<BiquadParams<float, 3>>();
            ButterworthFilterDesigner<float>::designSixPoleLowpass(*ret, normalizedFc);
            lowpass16 = ret;
        }
        return ret;
    }
    else assert(false);
    return nullptr;
}

// The weak pointers that hold our singletons.
template <typename T>
std::weak_ptr< BiquadParams<float, 3> >  ObjectCache<T>::lowpass64;
template <typename T>
std::weak_ptr< BiquadParams<float, 3> >  ObjectCache<T>::lowpass16;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::bipolarAudioTaper;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::audioTaper;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::sinLookupTable;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::exp2;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::exp2ExLow;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::exp2ExHigh;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::db2Gain;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::tanh5;

// Explicit instantiation, so we can put implementation into .cpp file
template class ObjectCache<double>;
template class ObjectCache<float>;
