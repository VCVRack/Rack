#pragma once

#include <cmath>
#include <random>
#include "rack.hpp"
#include "dsp/resampler.hpp"
#include "DSPEffect.hpp"

#define LAMBERT_W_THRESHOLD 10e-10
using namespace rack;

const static float TWOPI = (float) M_PI * 2;

namespace dsp {


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
    double r = 0.999;
    double xm1 = 0.f, ym1 = 0.f;

    DCBlocker(double r);

    /**
     * @brief Filter signal
     * @param x Input sample
     * @return Filtered output
     */
    double filter(double x);
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

    Noise() {}


    float nextFloat(float gain) {
        static std::default_random_engine e;
        static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
        return (float) dis(e) * gain;
    }
};


/**
 * @brief Simple oversampling class
 * @deprecated Use resampler instead
 */
template<int OVERSAMPLE, int CHANNELS>
struct OverSampler {

    struct Vector {
        float y0, y1;
    };

    Vector y[CHANNELS] = {};
    float up[CHANNELS][OVERSAMPLE] = {};
    float data[CHANNELS][OVERSAMPLE] = {};
    rack::Decimator<OVERSAMPLE, OVERSAMPLE> decimator[CHANNELS];
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


/** Generate chebyshev polynoms
 * @brief
 * @param x Input sample
 * @param A ?
 * @param order Polynom order
 * @return
 */
inline float chebyshev(float x, float A[], int order) {
    // To = 1
    // T1 = x
    // Tn = 2.x.Tn-1 - Tn-2
    // out = sum(Ai*Ti(x)) , i C {1,..,order}
    float Tn_2 = 1.0f;
    float Tn_1 = x;
    float Tn;
    float out = A[0] * Tn_1;

    for (int n = 2; n <= order; n++) {
        Tn = 2.0f * x * Tn_1 - Tn_2;
        out += A[n - 1] * Tn;
        Tn_2 = Tn_1;
        Tn_1 = Tn;
    }
    return out;
}


/**
 * @brief Signum function
 * @param x
 * @return
 */
inline double sign(double x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}


/**
 * @brief Lambert-W function using Halley's method
 *        see: http://smc2017.aalto.fi/media/materials/proceedings/SMC17_p336.pdf
 * @param x
 * @param ln1
 * @return
 */
inline double lambert_W(double x, double ln1) {
    double w;
    double p, r, s, err;
    double expw;

    // if (!isnan(ln1) || !isfinite(ln1)) ln1 = 0.;

    // initial guess, previous value
    w = ln1;

//    debug("x: %f  ln1: %f", x, ln1);

    // Haley's method (Sec. 4.2 of the paper)
    for (int i = 0; i < 100; i++) {
        expw = pow(M_E, w);

        p = w * expw - x;
        r = (w + 1.) * expw;
        s = (w + 2.) / (2. * (w + 1.));
        err = (p / (r - (p * s)));

        if (abs(err) < 10e-12) {
            break;
        }

        w = w - err;
    }

    return w;
}


/**
 * @brief
 *
 * This function evaluates the upper branch of the Lambert-W function for
 * real input x.
 *
 * Function written by F. Esqueda 2/10/17 based on implementation presented
 * by Darko Veberic - "Lambert W Function for Applications in Physics"
 * Available at https://arxiv.org/pdf/1209.0735.pdf
 *
 * @param x input
 * @return W(x)
 */
inline double lambert_W_Fritsch(double x) {
    double num, den;
    double w, w1, a, b, ia;
    double z, q, e;

    if (x < 0.14546954290661823) {
        num = 1 + 5.931375839364438 * x + 11.39220550532913 * x * x + 7.33888339911111 * x * x * x + 0.653449016991959 * x * x * x * x;
        den = 1 + 6.931373689597704 * x + 16.82349461388016 * x * x + 16.43072324143226 * x * x * x + 5.115235195211697 * x * x * x * x;

        w = x * num / den;
    } else if (x < 8.706658967856612) {
        num = 1 + 2.4450530707265568 * x + 1.3436642259582265 * x * x + 0.14844005539759195 * x * x * x +
              0.0008047501729130 * x * x * x * x;
        den = 1 + 3.4447089864860025 * x + 3.2924898573719523 * x * x + 0.9164600188031222 * x * x * x +
              0.05306864044833221 * x * x * x * x;

        w = x * num / den;
    } else {
        a = log(x);
        b = log(a);
        ia = 1. / a;
        w = a - b + (b * ia) * 0.5 * b * (b - 2.) * (ia * ia) + (1. / 6.) * (2. * b * b - 9. * b + 6.) * (ia * ia * ia);
    }


    for (int i = 0; i < 20; i++) {
        w1 = w + 1.;
        z = log(x) - log(w) - w;
        q = 2. * w1 * (w1 + (2. / 3.) * z);
        e = (z / w1) * ((q - z) / (q - 2. * z));

        if (abs(e) < 10e-12) {
            break;
        }
    }

    return w;
}

} // namespace dsp
