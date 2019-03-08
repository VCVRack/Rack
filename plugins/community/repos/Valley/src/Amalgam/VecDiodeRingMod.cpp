#include "VecDiodeRingMod.hpp"

VecDiode::VecDiode() {
    __zeros = _mm_set1_ps(0.f);
    __den = __zeros;

    _num = 0.f;
    _den = 1.f;
    _hA = 0.f;
    _hB = 0.f;

    float initPrecision = 0.01f;
    float precision = initPrecision;
    float direction = -1.f;

    float x = 0.f;
    float xMax = 0.999999f;
    float xMin = 0.999998f;
    for(auto i = 0; i < DSJ_VEC_DIODE_TABLE_N; ++i) {
        _vB = (float)i / (float)(DSJ_VEC_DIODE_TABLE_N - 1) * 0.75f;
        for(auto j = 0; j < DSJ_VEC_DIODE_TABLE_N; ++j) {
            _hB = 4.f;
            precision = initPrecision;
            direction = -1.f;
            _vL = rescale((float)j, 0.f, (float)(DSJ_VEC_DIODE_TABLE_N - 1), _vB, 1.f);
            _vL = clamp(_vL, _vB + 0.0001f, 1.f);
            while(true) {
                x = _hB * calcLin(1.f, _vB, _vL);
                if(x >= xMin && x <= xMax) {
                    break;
                }
                _hB += direction * precision;
                if(x < xMin && direction == -1.f) {
                    direction = 1.f;
                    precision *= 0.1f;
                }
                else if(x > xMax && direction == 1.f) {
                    direction = -1.f;
                    precision *= 0.1f;
                }
            }
            _makeupGain[i][j] = _hB;
        }
    }
}

__m128 VecDiode::process(__m128 x) {
    __outx = _mm_and_ps(vecCalcNLP(x, _vB, _vLScaled),_mm_cmpgt_ps(x, _mm_set1_ps(_vB)));
    __outx = _mm_switch_ps(__outx, vecCalcLin(x, _vB, _vLScaled), _mm_cmpgt_ps(x, _mm_set1_ps(_vLScaled)));
    return _mm_mul_ps(__outx, _mm_set1_ps(_hB));
}

void VecDiode::setV(float vB, float vL) {
    _vB = 0.75f * vB;
    _vL = clamp(vL, 0.0f, 1.f);
    _vLScaled = rescale(_vL, 0.0f, 1.f, _vB, 1.f);
    _vLScaled = clamp(_vLScaled, _vB + 0.001f, 1.f);
    __den = _mm_set1_ps(1.f / (2.f * (_vLScaled - _vB)));
    calcMakeupGain();
}

float VecDiode::calcNLP(float x, float vB, float vL) {
    float num = x - vB;
    num *= num;
    return num / (2 * (vL - vB));
}

float VecDiode::calcLin(float x, float vB, float vL) {
    float num = vL - vB;
    num *= num;
    return x - vL + num / (2 * (vL - vB));
}

__m128 VecDiode::vecCalcNLP(__m128 x, float vB, float vL) {
    __m128 num = _mm_sub_ps(x, _mm_set1_ps(vB));
    num = _mm_mul_ps(num, num);
    return _mm_mul_ps(num, __den);
}

__m128 VecDiode::vecCalcLin(__m128 x, float vB, float vL) {
    float num = vL - vB;
    num *= num;
    return _mm_add_ps(_mm_sub_ps(x, _mm_set1_ps(vL)), _mm_mul_ps(_mm_set1_ps(num), __den));
}

float VecDiode::subCalcLin(float vB, float vL) {
    float num = vL - vB;
    num *= num;
    return vL + num / (2 * (vL - vB));
}

void VecDiode::calcMakeupGain() {
    _vBF = rescale(_vB, 0.001f, 0.75f, 0.f, (float)DSJ_VEC_DIODE_TABLE_N - 1);
    _vLF = rescale(_vLScaled, _vB, 1.f, 0.f, (float)DSJ_VEC_DIODE_TABLE_N - 1);
    _vBI_1 = clamp((long)_vBF, 0, DSJ_VEC_DIODE_TABLE_N - 1);
    _vLI_1 = clamp((long)_vLF, 0, DSJ_VEC_DIODE_TABLE_N - 1);
    _vBI_2 = clamp(_vBI_1 + 1, 0, DSJ_VEC_DIODE_TABLE_N - 1);
    _vLI_2 = clamp(_vLI_1 + 1, 0, DSJ_VEC_DIODE_TABLE_N - 1);

    _vBF -= (float)_vBI_1;
    _vLF -= (float)_vLI_1;

    _lutA = _makeupGain[_vBI_1][_vLI_1];
    _lutB = _makeupGain[_vBI_1][_vLI_2];
    _lutC = _makeupGain[_vBI_2][_vLI_1];
    _lutD = _makeupGain[_vBI_2][_vLI_2];

    _hB = linterp(linterp(_lutA, _lutB, _vBF),
                  linterp(_lutC, _lutD, _vBF), _vLF);
}

__m128 VecDiodeRingMod::process(__m128 x, __m128 y, float vB, float vL) {
    d.setV(vB, vL);
    __b = _mm_mul_ps(x, _mm_set1_ps(0.5f));
    __a = _mm_add_ps(y, __b);
    __b = _mm_sub_ps(y, __b);
    __negA = _mm_mul_ps(_mm_set1_ps(-1.f), __a);
    __negB = _mm_mul_ps(_mm_set1_ps(-1.f), __b);
    return _mm_sub_ps(_mm_add_ps(d.process(__a), d.process(__negA)),
                      _mm_add_ps(d.process(__b), d.process(__negB)));
}
