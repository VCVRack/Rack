#pragma once
#include <assert.h>

extern int _numBiquads;
/**
 * Structure to hold the input parameter of a biquad filter: in this case the tap weights.
 *
 * T is the numeric type (float or double)
 * N is the number of stages. So N=2 could be for a three pole or four pole biquad)
 */
template <typename T, int N>
class BiquadParams
{
public:
    BiquadParams();
    ~BiquadParams();

    // TODO2: make N be unsigned

    // Let's block copies. They work fine, but we don't need
    // to copy.
    BiquadParams(const BiquadParams&) = delete;
    BiquadParams& operator=(const BiquadParams&) = delete;

    T& B0(int stage);
    T& B1(int stage);
    T& B2(int stage);
    T& A1(int stage);
    T& A2(int stage);
    T B0(int stage) const;
    T B1(int stage) const;
    T B2(int stage) const;
    T A1(int stage) const;
    T A2(int stage) const;

    void dump() const;
private:
    T _taps[5 * N];
};

template <typename T, int N>
inline void BiquadParams<T, N>::dump() const
{
    for (int stage = 0; stage < N; ++stage) {
        printf("%d B0=%f\n", stage, B0(stage));
        printf("%d B1=%f\n", stage, B1(stage));
        printf("%d B2=%f\n", stage, B2(stage));
        printf("%d A1=%f\n", stage, A1(stage));
        printf("%d A2=%f\n\n", stage, A2(stage));
    }
}

template <typename T, int N>
inline BiquadParams<T, N>::BiquadParams()
{
    assert(N > 0);
    for (int i = 0; i < N * 5; ++i) {
        _taps[i] = 0;
    }
    ++_numBiquads;
}

template <typename T, int N>
inline BiquadParams<T, N>::~BiquadParams()
{
    --_numBiquads;
}

template <typename T, int N>
inline T& BiquadParams<T, N>::B0(int stage)
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5];
}

template <typename T, int N>
inline T& BiquadParams<T, N>::B1(int stage)
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 1];
}

template <typename T, int N>
inline T& BiquadParams<T, N>::B2(int stage)
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 2];
}

template <typename T, int N>
T BiquadParams<T, N>::A1(int stage) const
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 3];
}

template <typename T, int N>
T BiquadParams<T, N>::A2(int stage) const
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 4];
}

template <typename T, int N>
T BiquadParams<T, N>::B0(int stage) const
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5];
}

template <typename T, int N>
T BiquadParams<T, N>::B1(int stage) const
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 1];
}

template <typename T, int N>
T BiquadParams<T, N>::B2(int stage) const
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 2];
}

template <typename T, int N>
T& BiquadParams<T, N>::A1(int stage)
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 3];
}

template <typename T, int N>
T& BiquadParams<T, N>::A2(int stage)
{
    assert(stage >= 0 && stage < N);
    return _taps[stage * 5 + 4];
}