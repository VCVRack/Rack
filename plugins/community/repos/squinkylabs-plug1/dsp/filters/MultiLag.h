#pragma once

#include "AudioMath.h"
#include "LookupTable.h"
#include "LowpassFilter.h"

#include <assert.h>
#include <cmath>
#include <xmmintrin.h>
#include <mmintrin.h>

#define _LLOOK
#define _LPSSE

/**
 * initial CPU = 3.0, 54.1 change freq every sample
 * 2.1, 8.6 with lookup and SSE
 */
template <int N>
class MultiLag
{
public:
    MultiLag()
    {
#ifdef _LPSSE
        const float onef = 1.f;
        kOne = _mm_load1_ps(&onef);
#endif
    }
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);

    /**
     * attack and release, using direct filter L values
     */
    void setAttackL(float l)
    {
        lAttack = _mm_set_ps1(l);
    }
    void setReleaseL(float l)
    {
        lRelease = _mm_set_ps1(l);
    }
    void setEnable(bool b)
    {
        enabled = b;
    }

    void step(const float * buffer);
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }

private:
    float memory[N] = {0};

#ifdef _LPSSE
    __m128 lAttack = {0};
    __m128 lRelease = {0};
    __m128 kOne = {1};
#else
    float lAttack = 0;
    float kAttack = 0;
    float lRelease = 0;
    float kRelease = 0;
#endif
    bool enabled = true;

#ifdef _LLOOK
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
#endif
};


#if defined(_LLOOK) && defined(_LPSSE)
template <int N>
inline void MultiLag<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    lAttack = _mm_set_ps1(ls);
}


template <int N>
inline void MultiLag<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    lRelease = _mm_set_ps1(ls);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag<N>::step(const float * input)
{
    if (!enabled) {
        for (int i = 0; i < N; i += 4) {
            __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
            _mm_storeu_ps(memory + i, input4);
        }
        return;
    }
    assert((N % 4) == 0);
    for (int i = 0; i < N; i += 4) {
        __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
        __m128 memory4 = _mm_loadu_ps(memory + i);
        __m128 cmp = _mm_cmpge_ps(input4, memory4);     //cmp has 11111 where >=, 0000 others

#if 0 // old way, with k
        __m128 ka = _mm_and_ps(cmp, kAttack);
        __m128 kr = _mm_andnot_ps(cmp, kRelease);
        __m128 k = _mm_or_ps(ka, kr);

        __m128 l = _mm_sub_ps(kOne, k);
#else
        __m128 la = _mm_and_ps(cmp, lAttack);
        __m128 lr = _mm_andnot_ps(cmp, lRelease);
        __m128 l = _mm_or_ps(la, lr);
        __m128 k = _mm_sub_ps(kOne, l);
#endif

        // now k and l have the correct a/r time constants

        __m128 temp = _mm_mul_ps(input4, k);
        memory4 = _mm_mul_ps(memory4, l);
        memory4 = _mm_add_ps(memory4, temp);
        _mm_storeu_ps(memory + i, memory4);
    }
}
#endif


//*************************************************************************
#if !defined(_LLOOK) && !defined(_LPSSE)
template <int N>
inline void MultiLag<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    lAttack = LowpassFilter<float>::computeLfromFs(fs);
    kAttack = LowpassFilter<float>::computeKfromL(lAttack);
}

template <int N>
inline void MultiLag<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    lRelease = LowpassFilter<float>::computeLfromFs(fs);
    kRelease = LowpassFilter<float>::computeKfromL(lRelease);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag<N>::step(const float * input)
{
    if (!enabled) {
        for (int i = 0; i < N; ++i) {
            memory[i] = input[i];
        }
        return;
    }
    for (int i = 0; i < N; ++i) {
        if (input[i] > memory[i]) {
            memory[i] = memory[i] * lAttack + kAttack * input[i];
        } else {
            memory[i] = memory[i] * lRelease + kRelease * input[i];
        }
    }
}
#endif

/******************************************************************************************************/

/**
 * initial CPU = 2.3, 28.1 change freq every sample
 *                  , 6.1 with non-uniform look
 *      1.2 with SSE version
 */
template <int N>
class MultiLPF
{
public:
    // normalized cutoff freq
    void setCutoff(float);

    void step(const float * buffer);

    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }

private:
    float memory[N] = {0};


#ifdef _LPSSE
    __m128 l = {0};
    __m128 k = {0};
#else
    float l = 0;
    float k = 0;
#endif

#ifdef _LLOOK
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
#endif
};


template <int N>
inline void MultiLPF<N>::setCutoff(float fs)
{
    assert(fs > 00 && fs < .5);
#if defined(_LLOOK) && !defined(_LPSSE)
       // k = NonUniformLookupTable<float>::lookup(*lookup, fs);
       // l = float(1.0 - k);
    assert(false);
#elif !defined(_LLOOK) && !defined(_LPSSE) 
    l = LowpassFilter<float>::computeLfromFs(fs);
    k = LowpassFilter<float>::computeKfromL(l);
#elif  defined(_LLOOK) && defined(_LPSSE)
    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    float ks = LowpassFilter<float>::computeKfromL(ls);
    k = _mm_set_ps1(ks);
    l = _mm_set_ps1(ls);
#else
    assert(false);
#endif

}


/**
 * z = _z * _l + _k * x;
 */
#if !defined(_LPSSE)
template <int N>
inline void MultiLPF<N>::step(const float * input)
{
    for (int i = 0; i < N; ++i) {
        memory[i] = memory[i] * l + k * input[i];
    }
}
#else
template <int N>
inline void MultiLPF<N>::step(const float * input)
{
    assert((N % 4) == 0);
    for (int i = 0; i < N; i += 4) {
        __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
        __m128 memory4 = _mm_loadu_ps(memory + i);

        __m128 temp = _mm_mul_ps(input4, k);
        memory4 = _mm_mul_ps(memory4, l);
        memory4 = _mm_add_ps(memory4, temp);
        _mm_storeu_ps(memory + i, memory4);
    }
}
#endif


/**
 * lookup table that goes from knob position (0..1) to
 * lowpass filter 'L' values.
 */

// derived by trial and error, at 44k
static float sampledKValues[] = {
    .4f,     // 0
    .4f,     // .1
    .044f,   // .2
    .03f,    // .3 guess
    .019f,   //4
    .004f,
    .0019f,  //6
    .0004f,
    .00011f, // 8
    .00004f,
    .00002f, // 10
    .00002f
};

// Computes filter "l" value for lookup table entries.
// Input comes from sampledKValuesk
static float getLValue(int index, float sampleTime)
{
    assert(index >= 0);
    assert(index <= 11);
    float _k = sampledKValues[index];
    float l = 1.0f - _k;
    float fs = (float) std::log(l) / (-2.0f *  (float) AudioMath::Pi);
    float  fTarget = fs * 44100;
    float fsAdjusted = fTarget * sampleTime;
    float ret = LowpassFilter<float>::computeLfromFs(fsAdjusted);
  //  printf("index = %i k=%f l=%f\n  fs=%f f=%f ret=%f\n", index, _k, l, fs, fTarget, ret);
    return ret;
}

template <typename T>
inline std::shared_ptr <LookupTableParams<T>> makeLPFDirectFilterLookup(float sampleTime)
{
    std::shared_ptr <LookupTableParams<T>> params = std::make_shared< LookupTableParams<T>>();
    LookupTable<T>::init(*params, 10, 0, 1, [sampleTime](double x) {
        int index = (int) std::round(x * 10);
        return getLValue(index, sampleTime);
        });
    return params;
}


