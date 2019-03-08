#include "DiodeRingMod.hpp"

Diode::Diode() {
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
    for(auto i = 0; i < DSJ_DIODE_TABLE_N; ++i) {
        _vB = (float)i / (float)(DSJ_DIODE_TABLE_N - 1) * 0.75f;
        for(auto j = 0; j < DSJ_DIODE_TABLE_N; ++j) {
            _hB = 4.f;
            precision = initPrecision;
            direction = -1.f;
            _vL = rescale((float)j, 0.f, (float)(DSJ_DIODE_TABLE_N - 1), _vB, 1.f);
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

float Diode::process(float x) {
    if(x <= _vB) {
        _out = 0.f;
    }
    else if(x > _vB && x <= _vLScaled) {
        _out = calcNLP(x, _vB, _vLScaled);
    }
    else {
        _out = calcLin(x, _vB, _vLScaled);
    }
    return _out * _hB;
}

void Diode::setV(float vB, float vL) {
    _vB = 0.75f * vB;
    _vL = clamp(vL, 0.0f, 1.f);
    _vLScaled = rescale(_vL, 0.0f, 1.f, _vB, 1.f);
    _vLScaled = clamp(_vLScaled, _vB + 0.001f, 1.f);
    calcMakeupGain();
}

float Diode::calcNLP(float x, float vB, float vL) {
    float num = x - vB;
    num *= num;
    return num / (2 * (vL - vB));
}

float Diode::calcLin(float x, float vB, float vL) {
    float num = vL - vB;
    num *= num;
    return x - vL + num / (2 * (vL - vB));
}

void Diode::calcMakeupGain() {
    _vBF = rescale(_vB, 0.001f, 0.75f, 0.f, (float)DSJ_DIODE_TABLE_N - 1);
    _vLF = rescale(_vLScaled, _vB, 1.f, 0.f, (float)DSJ_DIODE_TABLE_N - 1);
    _vBI_1 = clamp((long)_vBF, 0, DSJ_DIODE_TABLE_N - 1);
    _vLI_1 = clamp((long)_vLF, 0, DSJ_DIODE_TABLE_N - 1);
    _vBI_2 = clamp(_vBI_1 + 1, 0, DSJ_DIODE_TABLE_N - 1);
    _vLI_2 = clamp(_vLI_1 + 1, 0, DSJ_DIODE_TABLE_N - 1);

    _vBF -= (float)_vBI_1;
    _vLF -= (float)_vLI_1;

    _lutA = _makeupGain[_vBI_1][_vLI_1];
    _lutB = _makeupGain[_vBI_1][_vLI_2];
    _lutC = _makeupGain[_vBI_2][_vLI_1];
    _lutD = _makeupGain[_vBI_2][_vLI_2];

    _hB = linterp(linterp(_lutA, _lutB, _vBF),
                       linterp(_lutC, _lutD, _vBF), _vLF);
}

float DiodeRingMod::process(float x, float y, float vB, float vL) {
    d.setV(vB, vL);
    b = x * 0.5f;
    a = y + b;
    b = y - b;
    return d.process(a) + d.process(-a) - d.process(b) + d.process(-b);
}
