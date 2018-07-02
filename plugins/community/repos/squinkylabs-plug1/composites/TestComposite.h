#pragma once

#include <vector>

/**
* Base class for composites embeddable in a unit test
* Isolates test from VCV.
*/

class TestComposite
{
public:
    TestComposite() :
        inputs(20),
        outputs(20),
        params(20),
        lights(20)
    {

    }
    struct Param
    {
        float value = 0.0;
    };

    struct Light
    {
        /** The square of the brightness value */
        float value = 0.0;
        float getBrightness();
        void setBrightness(float brightness)
        {
            value = (brightness > 0.f) ? brightness * brightness : 0.f;
        }
        void setBrightnessSmooth(float brightness);
    };

    struct Input
    {
        /** Voltage of the port, zero if not plugged in. Read-only by Module */
        float value = 0.0;
        /** Whether a wire is plugged in */
        bool active = false;
        Light plugLights[2];
        /** Returns the value if a wire is plugged in, otherwise returns the given default value */
        float normalize(float normalValue)
        {
            return active ? value : normalValue;
        }
    };

    struct Output
    {
        /** Voltage of the port. Write-only by Module */
        float value = 0.0;
        /** Whether a wire is plugged in */
        bool active = false;
        Light plugLights[2];
    };

    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Param> params;
    std::vector<Light> lights;
};
