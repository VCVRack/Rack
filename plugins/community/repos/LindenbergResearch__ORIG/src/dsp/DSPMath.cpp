#include "DSPMath.hpp"

/**
 * @brief Clip signal at bottom by value
 * @param in Sample input
 * @param clip Clipping value
 * @return Clipped sample
 */
float clipl(float in, float clip) {
    if (in < clip) return clip;
    else return in;
}


/**
 * @brief Clip signal at top by value
 * @param in Sample input
 * @param clip Clipping value
 * @return Clipped sample
 */
float cliph(float in, float clip) {
    if (in > clip) return clip;
    else return in;
}


/**
 * @brief Wrap input number between -PI..PI
 * @param n Input number
 * @return Wrapped value
 */
float wrapTWOPI(float n) {
    float b = 1.f / TWOPI * n;
    return (b - lround(b)) * TWOPI;
}


/**
 * @brief Get PLL increment depending on frequency
 * @param frq Frequency
 * @return  PLL increment
 */
float getPhaseIncrement(float frq) {
    return TWOPI * frq / engineGetSampleRate();
}


/**
 * @brief Actual BLIT core computation
 * @param N Harmonics
 * @param phase Current phase value
 * @return
 */
float BLITcore(float N, float phase) {
    float a = wrapTWOPI((clipl(N - 1, 0.f) + 0.5f) * phase);
    float x = fastSin(a) * 1.f / fastSin(0.5f * phase);
    return (x - 1.f) * 2.f;
}


/**
 * @brief BLIT generator based on current phase
 * @param N Harmonics
 * @param phase Current phase of PLL
 * @return
 */
float BLIT(float N, float phase) {
    if (phase == 0.f) return 1.f;
    else return BLITcore(N, phase);
}


/**
 * @brief Add value to integrator
 * @param x Input
 * @param Fn
 * @return
 */
float Integrator::add(float x, float Fn) {
    value = (x - value) * (d * Fn) + value;
    return value;
}


/**
 * @brief Filter function for DC block
 * @param x Input sample
 * @return Filtered sample
 */
float DCBlocker::filter(float x) {
    float y = x - xm1 + R * ym1;
    xm1 = x;
    ym1 = y;

    return y;
}


/**
 * @brief Filter function for simple 6dB lowpass filter
 * @param x Input sample
 * @return
 */
float LP6DBFilter::filter(float x) {
    float y = y0 + (alpha * (x - y0));
    y0 = y;

    return y;
}


/**
 * @brief Update filter parameter
 * @param fc Cutoff frequency
 */
void LP6DBFilter::updateFrequency(float fc, int factor) {
    LP6DBFilter::fc = fc;
    RC = 1.f / (LP6DBFilter::fc * TWOPI);
    dt = 1.f / engineGetSampleRate() * factor;
    alpha = dt / (RC + dt);
}


/**
 * @brief Shaper type 1 (Saturate)
 * @param a Amount from 0 - x
 * @param x Input sample
 * @return
 */
float shape1(float a, float x) {
    float k = 2 * a / (1 - a);
    float b = (1 + k) * (x * 0.5f) / (1 + k * fabsf(x * 0.5f));

    return b * 4;
}


/**
 * @brief Waveshaper as used in ReShaper. Input should be in the range -1..+1
 * @param a Shaping factor
 * @param x Input sample
 * @return
 */
float shape2(float a, float x) {
    return atanf(x * a);//x * (fabs(x) + a) / (x * x + (a - 1) * fabs(x) + 1);
}

/**
 * @brief Soft saturating with a clip of a. Works only with positive values so use 'b' as helper here.
 * @param x Input sample
 * @param a Saturating threshold
 * @return
 */
double saturate(double x, double a) {
    double b = 1;

    /* turn negative values positive and remind in b as coefficient */
    if (x < 0) {
        b = -1;
        x *= -1;
    }

    // nothing to do
    if (x <= a) return x * b;

    double d = (a + (x - a) / (1 + pow((x - a) / (1 - a), 2)));

    if (d > 1) {
        return (a + 1) / 2 * b;
    } else {
        return d * b;
    }
}


/**
 * @brief
 * @param input
 * @return
 */
double overdrive(double input) {
    const double x = input * 0.686306;
    const double a = 1 + exp(sqrt(fabs(x)) * -0.75);
    return (exp(x) - exp(-x * a)) / (exp(x) + exp(-x));
}