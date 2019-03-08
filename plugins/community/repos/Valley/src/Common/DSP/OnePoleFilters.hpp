#pragma once
#include <cmath>
#include <stdio.h>
#define _1_FACT_2 0.5
#define _1_FACT_3 0.1666666667
#define _1_FACT_4 0.04166666667
#define _1_FACT_5 0.008333333333
#define _2M_PI 2.0 * M_PI

template<typename T>
T fastexp(T x) {
    T xx = x * x;
    T x3 = x * xx;
    T x4 = xx * xx;
    T x5 = x4 * x;
    x = 1 + x + (xx * _1_FACT_2) + (x3 * _1_FACT_3) + (x4 * _1_FACT_4);
    return x + (x5 * _1_FACT_5);
}

class OnePoleLPFilter {
public:
    OnePoleLPFilter();
    OnePoleLPFilter(double cutoffFreq);
    double process();
    void clear();
    void setCutoffFreq(double cutoffFreq);
    void setSampleRate(double sampleRate);
    double getMaxCutoffFreq() const;
    double input;
    double output;
private:
    double _sampleRate;
    double _1_sampleRate;
    double _cutoffFreq;
    double _maxCutoffFreq;
    double _a;
    double _b;
    double _z;
};

class OnePoleHPFilter {
public:
    OnePoleHPFilter();
    OnePoleHPFilter(double cutoffFreq);
    double process();
    void clear();
    void setCutoffFreq(double cutoffFreq);
    void setSampleRate(double sampleRate);
    double input;
    double output;
private:
    double _sampleRate;
    double _1_sampleRate;
    double _cutoffFreq;
    double _y0;
    double _y1;
    double _x0;
    double _x1;
    double _a0;
    double _a1;
    double _b1;
};

class DCBlocker {
public:
    static int i;
    int id;
    DCBlocker();
    DCBlocker(double cutoffFreq);
    double process(double input);
    void clear();
    void setCutoffFreq(double cutoffFreq);
    void setSampleRate(double sampleRate);
    double getMaxCutoffFreq() const;
    double output;
private:
    double _sampleRate;
    double _cutoffFreq;
    double _maxCutoffFreq;
    double _b;
    double _z;
};
