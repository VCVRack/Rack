
#include <assert.h>

#include "AudioMath.h"
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
std::shared_ptr<LookupTableParams<T>> ObjectCache<T>::getSinLookup()
{
    std::shared_ptr< LookupTableParams<T>> ret = sinLookupTable.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<T>>();
        std::function<double(double)> f = AudioMath::makeFunc_Sin();
        // Used to use 4096, but 256 gives about 80db  snr, so let's save memory
        LookupTable<T>::init(*ret, 256, 0, 1, f);
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
        LookupTable<T>::init(*ret, 256, -5, 5,  [](double x) {
            return std::tanh(x);
            });
        tanh5 = ret;
    }
    return ret;
}

// The weak pointer that hold our singletons.
template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::bipolarAudioTaper;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::sinLookupTable;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::exp2;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::db2Gain;

template <typename T>
std::weak_ptr<LookupTableParams<T>> ObjectCache<T>::tanh5;

// Explicit instantiation, so we can put implementation into .cpp file
template class ObjectCache<double>;
template class ObjectCache<float>;
