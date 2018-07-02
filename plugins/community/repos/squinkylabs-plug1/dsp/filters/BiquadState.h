
#pragma once

/**
 * Structure to hold the mutable state of a biquad filter: in this case the delay memory.
 *
 * T is the numeric type (float or double)
 * N is the number of stages. So N=2 could be for a three pole or four pole biquad)
 */
template <typename T, int N>
class BiquadState
{
public:
    BiquadState();
    ~BiquadState();
    /**
     * accessor for delay memory 0
     * @param stage is the biquad stage index
     */
    T& z0(int stage);
    T& z1(int stage);
private:
    T _state[N * 2];
};

template <typename T, int N>
inline BiquadState<T, N>::BiquadState()
{
    assert(N > 0);
    for (int i = 0; i < N * 2; ++i) {
        _state[i] = 0;
    }
    _numBiquads++;
}

template <typename T, int N>
inline BiquadState<T, N>::~BiquadState()
{
    _numBiquads--;
}

template <typename T, int N>
inline T& BiquadState<T, N>::z0(int stage)
{
    assert(stage >= 0 && stage < N);
    return _state[stage * 2];
}

template <typename T, int N>
inline T& BiquadState<T, N>::z1(int stage)
{
    assert(stage >= 0 && stage < N);
    return _state[stage * 2 + 1];
}
