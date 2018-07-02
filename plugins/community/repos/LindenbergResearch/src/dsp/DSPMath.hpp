#pragma once

#include <cmath>
#include <random>
#include "rack.hpp"
#include "dsp/decimator.hpp"

using namespace rack;

const static float TWOPI = (float) M_PI * 2;


/**
 * @brief Basic leaky integrator
 */
struct Integrator {
    float d = 0.25f;
    float value = 0.f;

    /**
     * @brief Add value to integrator
     * @param x Input sample
     * @param Fn
     * @return Current integrator state
     */
    float add(float x, float Fn);
};


/**
 * @brief Filter out DC offset / 1-Pole HP Filter
 */
struct DCBlocker {
    const float R = 0.999;
    float xm1 = 0.f, ym1 = 0.f;

    /**
     * @brief Filter signal
     * @param x Input sample
     * @return Filtered output
     */
    float filter(float x);
};


/**
 * @brief Simple 6dB lowpass filter
 */
struct LP6DBFilter {
private:
    float RC;
    float dt;
    float alpha;
    float y0;
    float fc;

public:

    /**
     * @brief Create a new filter with a given cutoff frequency
     * @param fc cutoff frequency
     * @param factor Oversampling factor
     */
    LP6DBFilter(float fc, int factor) {
        updateFrequency(fc, factor);
        y0 = 0.f;
    }


    /**
     * @brief Set new cutoff frequency
     * @param fc cutoff frequency
     */
    void updateFrequency(float fc, int factor);

    /**
     * @brief Filter signal
     * @param x Input sample
     * @return Filtered output
     */
    float filter(float x);
};


/**
 * @brief Simple noise generator
 */
struct Noise {

    Noise() {

    }


    float nextFloat(float gain) {
        static std::default_random_engine e;
        static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
        return (float) dis(e) * gain;
    }
};


/**
 * @brief Simple oversampling class
 */
template<int OVERSAMPLE, int CHANNELS>
struct OverSampler {

    struct Vector {
        float y0, y1;
    };

    Vector y[CHANNELS] = {};
    float up[CHANNELS][OVERSAMPLE] = {};
    float data[CHANNELS][OVERSAMPLE] = {};
    Decimator<OVERSAMPLE, OVERSAMPLE> decimator[CHANNELS];
    int factor = OVERSAMPLE;


    /**
     * @brief Constructor
     * @param factor Oversampling factor
     */
    OverSampler() {}


    /**
     * @brief Return linear interpolated position
     * @param point Point in oversampled data
     * @return
     */
    float interpolate(int channel, int point) {
        return y[channel].y0 + (point / factor) * (y[channel].y1 - y[channel].y0);
    }


    /**
     * @brief Create up-sampled data out of two basic values
     */
    void doUpsample(int channel) {
        for (int i = 0; i < factor; i++) {
            up[channel][i] = interpolate(channel, i + 1);
        }
    }


    /**
     * @brief Downsample data from a given channel
     * @param channel Channel to proccess
     * @return Downsampled point
     */
    float getDownsampled(int channel) {
        return decimator[channel].process(data[channel]);
    }


    /**
     * @brief Step to next sample point
     * @param y Next sample point
     */
    void next(int channel, float n) {
        y[channel].y0 = y[channel].y1;
        y[channel].y1 = n;
    }
};


/**
 * @brief Fast sin approximation
 * @param angle Angle
 * @return App. value
 */
inline float fastSin(float angle) {
    float sqr = angle * angle;
    float result = -2.39e-08f;
    result *= sqr;
    result += 2.7526e-06f;
    result *= sqr;
    result -= 1.98409e-04f;
    result *= sqr;
    result += 8.3333315e-03f;
    result *= sqr;
    result -= 1.666666664e-01f;
    result *= sqr;
    result += 1.0f;
    result *= angle;
    return result;
}


float wrapTWOPI(float n);

float getPhaseIncrement(float frq);

float clipl(float in, float clip);

float cliph(float in, float clip);

float BLIT(float N, float phase);

float shape1(float a, float x);

double saturate(double x, double a);

double overdrive(double input);

float shape2(float a, float x);


/**
 * @brief Double version of clamp
 * @param x
 * @param min
 * @param max
 * @return
 */
inline double clampd(double x, double min, double max) {
    return fmax(fmin(x, max), min);
}


/**
 * @brief Soft clipping
 * @param x
 * @param sat
 * @param satinv
 * @return
 */
inline float clip(float x, float sat, float satinv) {
    float v2 = (x * satinv > 1 ? 1 :
                (x * satinv < -1 ? -1 :
                 x * satinv));
    return (sat * (v2 - (1.f / 3.f) * v2 * v2 * v2));
}


/**
 * @brief Clamp without branching
 * @param input Input sample
 * @return
 */
inline double saturate2(double input) { //clamp without branching
    const double _limit = 0.3;
    double x1 = fabs(input + _limit);
    double x2 = fabs(input - _limit);
    return 0.5 * (x1 - x2);
}


/**
 * @brief Fast arctan approximation, corresponds to tanhf() but decreases y to infinity
 * @param x
 * @return
 */
inline float fastatan(float x) {
    return (x / (1.0f + 0.28f * (x * x)));
}


/**
 * @brief Linear fade of two points
 * @param a Point 1
 * @param b Point 2
 * @param n Fade value
 * @return
 */
inline float fade2(float a, float b, float n) {
    return (1 - n) * a + n * b;
}


/**
 * @brief Linear fade of five points
 * @param a Point 1
 * @param b Point 2
 * @param c Point 3
 * @param d Point 4
 * @param e Point 5
 * @param n Fade value
 * @return
 */
inline float fade5(float a, float b, float c, float d, float e, float n) {
    if (n >= 0 && n < 1) {
        return fade2(a, b, n);
    } else if (n >= 1 && n < 2) {
        return fade2(b, c, n - 1);
    } else if (n >= 2 && n < 3) {
        return fade2(c, d, n - 2);
    } else if (n >= 3 && n < 4) {
        return fade2(d, e, n - 3);
    }

    return e;
}


/**
* @brief Shapes between 0..1 with a log type courve
* @param x
* @return
*/
inline float cubicShape(float x) {
    return (x - 1.f) * (x - 1.f) * (x - 1.f) + 1.f;
}


/**
 * @brief ArcTan like shaper for foldback distortion
 * @param x
 * @return
 */
inline float atanShaper(float x) {
    return x / (1.f + (0.28f * x * x));
}