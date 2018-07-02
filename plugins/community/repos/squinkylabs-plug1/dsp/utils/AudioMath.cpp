#include <assert.h>
#include <memory>

#include "AudioMath.h"
#include "LookupTable.h"
#include "ObjectCache.h"

const double AudioMath::Pi = 3.1415926535897932384626433832795028841971;
const double AudioMath::Pi_2 = 1.5707963267948966192313216916397514420986;
const double AudioMath::Ln2 = 0.693147180559945309417;
const double AudioMath::Ln10 = 2.30258509299404568402;
const double AudioMath::E = 2.71828182845904523536;

std::function<double(double)> AudioMath::makeFunc_Sin()
{
    return [](double x) {
        return std::sin(x * 2 * Pi);
    };
}

std::function<double(double)> AudioMath::makeFunc_Exp(double xMin, double xMax, double yMin, double yMax)
{
    const double a = (std::log(yMax) - log(yMin)) / (xMax - xMin);
    const double b = log(yMin) - a * xMin;
    return [a, b](double d) {
        return std::exp(a * d + b);
    };
}

std::function<double(double)> AudioMath::makeFunc_AudioTaper(double dbAtten)
{
    assert(dbAtten < 0);

    const double gainAtQuarter = gainFromDb(dbAtten);
    std::function<double(double)> linearFunc;
    std::function<double(double)> expFunc;

    {
        // for linear part at bottom
        const double x0 = 0;
        const double x1 = .25;
        const double y0 = 0;
        const double y1 = gainAtQuarter;
        const double a = (y1 - y0) / (x1 - x0);
        const double b = y0 - a * x0;
        linearFunc = [a, b](double d) {
            return a * d + b;
        };
    }

    {
        // for exp part on top
        const double xMin = .25;
        const double yMin = gainAtQuarter;
        const double xMax = 1;
        const double yMax = 1;
        expFunc = makeFunc_Exp(xMin, xMax, yMin, yMax);
    }

    return [linearFunc, expFunc](double d) {
        return (d <= .25) ? linearFunc(d) : expFunc(d);
    };
}

AudioMath::ScaleFun<float> AudioMath::makeBipolarAudioScaler(float y0, float y1)
{
    // Use a cached singleton for the lookup table - don't need to have unique copies
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getBipolarAudioTaper();
    const float x0 = -5;
    const float x1 = 5;
    const float a = (y1 - y0) / (x1 - x0);
    const float b = y0 - a * x0;

    // Notice how lambda captures the smart pointer. Now
    // lambda owns a reference to it.
    return [a, b, lookup](float cv, float knob, float trim) {
        auto mappedTrim = LookupTable<float>::lookup(*lookup, trim);
        float x = cv * mappedTrim + knob;
        x = std::max<float>(-5.0f, x);
        x = std::min(5.0f, x);
        return a * x + b;
    };
}

 // declare some test variables here
int _numLookupParams = 0;
int _numBiquads = 0;