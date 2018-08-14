#include "MS20zdf.hpp"

using namespace dsp;


/**
 * @brief Calculate prewarped vars on parameter change
 */
void MS20zdf::invalidate() {
    // translate frequency to logarithmic scale
    //  freqHz = 20.f * powf(860.f, param[FREQUENCY].value) - 20.f;
    freqHz = 20.f * powf(950.f, param[FREQUENCY].value) - 20.f;

    b = tanf(freqHz * (float) M_PI / sr / OVERSAMPLE);
    g = b / (1 + b);

    /* use shifted negative cubic shape for logarithmic like shaping of the peak parameter */
    k = 2.f * cubicShape(param[PEAK].value) * 1.0001f;
    g2 = g * g;
}


/**
 * @brief Proccess one sample of filter
 */
void MS20zdf::process() {
    os.next(IN, input[IN].value);
    os.doUpsample(IN);

    float s1, s2;
    float gain = quadraticBipolar(param[DRIVE].value) * DRIVE_GAIN + 1.f;
    float type = param[TYPE].value;
    float x = 0;

    for (int i = 0; i < os.factor; i++) {
        x = os.up[IN][i];

        zdf1.set(x - ky, g);
        s1 = zdf1.s;

        zdf2.set(zdf1.y + ky, g);
        s2 = zdf2.s;

        y = 1.f / (g2 * k - g * k + 1.f) * (g2 * x + g * s1 + s2);

        ky = k * atanf(y / 70.f) * 70.f;

        if (type > 0) {
            os.data[IN][i] = atanShaper(gain * y / 10.f) * 10.f;
        } else {
            os.data[IN][i] = atanf(gain * y / 10.f) * 10.f;

        }
    }

    float out = os.getDownsampled(IN);

    output[OUT].value = out;
}


/**
 * @brief Inherit constructor
 * @param sr sample rate
 */
MS20zdf::MS20zdf(float sr) : DSPSystem(sr) {}

