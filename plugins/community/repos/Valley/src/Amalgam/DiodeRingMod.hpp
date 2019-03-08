#pragma once
#include <iostream>
#include "../Valley.hpp"
#include "../Common/Utilities.hpp"
#define DSJ_DIODE_TABLE_N 512
using namespace std;

class Diode {
public:
    Diode();
    float process(float x);
    void setV(float vB, float vL);
private:
    float _out, _num, _den;
    float _vB, _vL, _vLScaled;
    float _hA, _hB;
    float _vBF, _vLF;
    int _vBI_1, _vLI_1, _vBI_2, _vLI_2;
    float _lutA, _lutB, _lutC, _lutD;
    float _makeupGain[512][512];
    float calcNLP(float x, float vB, float vL);
    float calcLin(float x, float vB, float vL);
    void calcMakeupGain();
};

class DiodeRingMod {
public:
    float process(float x, float y, float vB, float vL);
private:
    float a;
    float b;
    Diode d;
};
