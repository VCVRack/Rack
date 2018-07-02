#pragma once
#include <cmath>
#include <functional>
#include <algorithm>

class AudioMath
{
public:
    AudioMath() = delete;       // we are only static
    static const double Pi;
    static const double Pi_2;       // Pi / 2
    static const double Ln2;
    static const double Ln10;
    static const double E;

    static bool closeTo(double x, double y, double tolerance)
    {
        const bool ret = std::abs(x - y) < tolerance;
        return ret;
    }

    static double db(double g)
    {
        return 20 * log(g) / Ln10;
    }

    static double gainFromDb(double db)
    {
        return std::exp(Ln10 * db / 20.0);
    }

    /**
     * Returns a function that generates one period of sin for x = {0..1}.
     * Range (output) is -1 to 1.
     */
    static std::function<double(double)> makeFunc_Sin();

    /*
     * Returns a function that generates an exponential defined by two points
     * At input = xMin, output will be yMin.
     * At input = xMax, output will be yMax.
     */
    static std::function<double(double)> makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);

    /**
     * Returns a function for an "audio taper" attenuator gain.
     * function is pure exponential for x > .25, linear for x < .25
     * At input 1, output is 1
     * At input .25, output is the gain corresponding to adAtten
     * At input 0, the output is zero.
     */
    static std::function<double(double)> makeFunc_AudioTaper(double dbAtten);

    /**
     * ScaleFun is a function the combines CV, knob, and trim into a voltage.
     * Typically a ScaleFun is like an "attenuverter"
     */
    template <typename T>
    using ScaleFun = std::function<T(T cv, T knob, T trim)>;

    /**
     * Create a ScaleFun with the following properties:
     * 1) The values are combined with the typical formula: x = cv * trim + knob;
     * 2) x is clipped between -5 and 5
     * 3) range is then interpolated between y0, and y1.
     *
     * This particular function is used when knobs are -5..5,
     * and CV range is -5..5.
     */
    template <typename T>
    static ScaleFun<T> makeLinearScaler(T y0, T y1)
    {
        const T x0 = -5;
        const T x1 = 5;
        const T a = (y1 - y0) / (x1 - x0);
        const T b = y0 - a * x0;
        return [a, b](T cv, T knob, T trim) {
            T x = cv * trim + knob;
            x = std::max<T>(-5.0f, x);
            x = std::min(5.0f, x);
            return a * x + b;
        };
    }

    /**
     * Generates a scale function for an audio taper attenuverter.
     * Details the same as makeLinearScaler except that the CV
     * scaling will be exponential for most values, becoming
     * linear near zero.
     */
    static ScaleFun<float> makeBipolarAudioScaler(float y0, float y1);
};
