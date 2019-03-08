#pragma once
template <typename T, int order>
class Poly
{
public:
    Poly();
    float run(float x, float inputGain);
    void setGain(int index, float value)
    {
        assert(index >= 0 && index < order);
        gains[index] = value;
    }

    T getOutput(int index) const
    {
        return outputs[index];
    }

private:
    T gains[order];

    // powers[0] = x, powers[1] = x**2
    T powers[order];

    T outputs[order];

    // since DC offset changes with gain, we need to compute it.
    static const int numEvenHarmonics = order / 2;
    T dcComponent[numEvenHarmonics];

    void fillPowers(T);
    T doSum();
    void calcDC(T gain);
};

template <typename T, int order>
inline Poly<T, order>::Poly()
{
    assert(order == 10);        // Although this class it templated, internally it's hard coded.
    for (int i = 0; i < order; ++i) {
        gains[i] = 0;
        powers[i] = 0;
        outputs[i] = 0;
    }

    for (int i = 0; i < numEvenHarmonics; ++i) {
        dcComponent[i] = 0;
    }
}

template <typename T, int order>
inline float Poly<T, order>::run(float x, float inputGain)
{
    fillPowers(x);
    calcDC(inputGain);
    return (float) doSum();
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
    outputs[0] =  powers[0];
    T sum = outputs[0] * gains[0];

    outputs[1] = (2 * powers[1] - dcComponent[0]);
    sum += outputs[1] * gains[1];

    outputs[2] =  (4 * powers[2] - 3 * powers[0]);
    sum += outputs[2] * gains[2];

    outputs[3] =  (8 * powers[3] - 8 * powers[1] - dcComponent[1]);
    sum += outputs[3] * gains[3];

    outputs[4] =  (16 * powers[4] - 20 * powers[2] + 5 * powers[0]);
    sum += outputs[4] * gains[4];

    outputs[5] =  (32 * powers[5] - 48 * powers[3] + 18 * powers[1] - dcComponent[2]);
    sum += outputs[5] * gains[5];

    outputs[6] =  (64 * powers[6] - 112 * powers[4] + 56 * powers[2] - 7 * powers[0]);
    sum += outputs[6] * gains[6];

    outputs[7] =  (128 * powers[7] - 256 * powers[5] + 160 * powers[3] - 32 * powers[1] - dcComponent[3]);
    sum += outputs[7] * gains[7];

    outputs[8] =  (256 * powers[8] - 576 * powers[6] + 432 * powers[4] - 120 * powers[2] + 9 * powers[0]);
    sum += outputs[8] * gains[8];

    outputs[9] =  (512 * powers[9] - 1280 * powers[7] + 1120 * powers[5] - 400 * powers[3] + 50 * powers[1] - dcComponent[4]);
    sum += outputs[9] * gains[9];

    return sum;
}
#if 0 // old way
template <typename T, int order>
inline T Poly<T, order>::doSum()
{
    T ret = gains[0] * powers[0];
    ret += gains[1] * (2 * powers[1] - dcComponent[0]);
    ret += gains[2] * (4 * powers[2] - 3 * powers[0]);
    ret += gains[3] * (8 * powers[3] - 8 * powers[1] - dcComponent[1]);
    ret += gains[4] * (16 * powers[4] - 20 * powers[2] + 5 * powers[0]);
    ret += gains[5] * (32 * powers[5] - 48 * powers[3] + 18 * powers[1] - dcComponent[2]);
    ret += gains[6] * (64 * powers[6] - 112 * powers[4] + 56 * powers[2] - 7 * powers[0]);
    ret += gains[7] * (128 * powers[7] - 256 * powers[5] + 160 * powers[3] - 32 * powers[1] - dcComponent[3]);
    ret += gains[8] * (256 * powers[8] - 576 * powers[6] + 432 * powers[4] - 120 * powers[2] + 9 * powers[0]);
    ret += gains[9] * (512 * powers[9] - 1280 * powers[7] + 1120 * powers[5] - 400 * powers[3] + 50 * powers[1] - dcComponent[4]);
    return ret;
}
#endif

template <typename T, int order>
void Poly<T, order>::calcDC(T gain)
{
    // first calculate "sinEnergy", which is the integral of sin^n(gain * x) over a period
    const double W2 = 2.0 / 4.0;
    const double W4 = W2 * 3.0 / 4.0;
    const double W6 = W4 * 5.0 / 6.0;
    const double W8 = W6 * 7.0 / 8.0;
    const double W10 = W8 * 9.0 / 10.0;

    T sinEnergy = gain;
    T sinEnergy2 = sinEnergy * sinEnergy;
    T sinEnergy4 = sinEnergy2 * sinEnergy2;
    T sinEnergy6 = sinEnergy4 * sinEnergy2;
    T sinEnergy8 = sinEnergy6 * sinEnergy2;
    T sinEnergy10 = sinEnergy8 * sinEnergy2;

    sinEnergy2 *= W2;
    sinEnergy4 *= W4;
    sinEnergy6 *= W6;
    sinEnergy8 *= W8;
    sinEnergy10 *= W10;

    // for harmonic 1 (first even)
    dcComponent[0] = T(2 * sinEnergy2);
    dcComponent[1] = T(8 * sinEnergy4 - 8 * sinEnergy2);
    dcComponent[2] = T(32 * sinEnergy6 - 48 * sinEnergy4 + 18 * sinEnergy2);
    dcComponent[3] = T(128 * sinEnergy8 - 256 * sinEnergy6 + 160 * sinEnergy4  - 32 * sinEnergy2);
    dcComponent[4] = T(512 * sinEnergy10 - 1280 * sinEnergy8 + 1120 * sinEnergy6 - 400 * sinEnergy4 + 50 * sinEnergy2);
}