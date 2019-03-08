#pragma once
#include <iostream>
#include "../Valley.hpp"
#include "../Common/Utilities.hpp"
#include "../Common/SIMD/SIMDUtilities.hpp"
#define DSJ_VEC_DIODE_TABLE_N 512
using namespace std;

class VecDiode {
public:
    VecDiode();
    __m128 process(__m128 x);
    void setV(float vB, float vL);
private:
    __m128 __zeros;
    __m128 __outx;
    __m128 __den;

    float _num, _den;
    float _vB, _vL, _vLScaled;
    float _hA, _hB;
    float _vBF, _vLF;
    int _vBI_1, _vLI_1, _vBI_2, _vLI_2;
    float _lutA, _lutB, _lutC, _lutD;
    float _makeupGain[512][512];
    float calcNLP(float x, float vB, float vL);
    float calcLin(float x, float vB, float vL);
    __m128 vecCalcNLP(__m128 x, float vB, float vL);
    __m128 vecCalcLin(__m128 x, float vB, float vL);
    float subCalcLin(float vB, float vL);
    void calcMakeupGain();
};

class VecDiodeRingMod {
public:
    __m128 process(__m128 x, __m128 y, float vB, float vL);
private:
    __m128 __a, __negA;
    __m128 __b, __negB;
    VecDiode d;
};
