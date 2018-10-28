#pragma once
template <typename T, int order>
class Poly
{
public:
    Poly();
    float run(float x)
    {
        fillPowers(x);
        return (float) doSum();
    }
    void setGain(int index, float value)
    {
        assert(index >= 0 && index < order);
        gains[index] = value;
    }
private:
    T gains[order];

    // powers[0] = x, powers[1] = x**2
    T powers[order];

    void fillPowers(T);
    T doSum();
};

template <typename T, int order>
inline Poly<T, order>::Poly()
{
    assert(order == 11);
    for (int i = 0; i < order; ++i) {
        gains[i] = 0;
        powers[i] = 0;
    }
}

template <typename T, int order>
inline void Poly<T, order>::fillPowers(T x)
{
    T value = x;
    for (int i = 0; i < order; ++i) {
        powers[i] = value;
        value *= x;
    }
}



template <typename T, int order>
inline T Poly<T, order>::doSum()
{
    T ret = gains[0] * powers[0];
    ret += gains[1] * (2 * powers[1] - 1);
    ret += gains[2] * (4 * powers[2] - 3 * powers[0]);
    ret += gains[3] * (8 * powers[3] - 8 * powers[1] + 1);
    ret += gains[4] * (16 * powers[4] - 20 * powers[2] + 5 * powers[0]);
    ret += gains[5] * (32 * powers[5] - 48 * powers[3] + 18 * powers[1] - 1);
    ret += gains[6] * (64 * powers[6] - 112 * powers[4] + 56 * powers[2] - 7 * powers[0]);
    ret += gains[7] * (128 * powers[7] - 256 * powers[5] + 160 * powers[3] - 32 * powers[1] + 1);
    ret += gains[8] * (256 * powers[8] - 576 * powers[6] + 432 * powers[4] - 120 * powers[2] + 9 * powers[0]);
    ret += gains[9] * (512 * powers[9] - 1280 * powers[7] + 1120 * powers[5] - 400 * powers[3] + 50 * powers[1] - 1);
    ret += gains[10] * (1024 * powers[10] - 2816 * powers[8] + 2816 * powers[6] - 1232 * powers[4] + 220 * powers[2] - 11 * powers[0]);
    return ret;
}