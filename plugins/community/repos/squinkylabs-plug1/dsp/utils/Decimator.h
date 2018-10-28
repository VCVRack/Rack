#pragma once

/**
 * Outputs a stair-step decimation.
 * So maybe it's the integral or a decimation?
 */
class Decimator
{
public:

    /**
     * ret the next sample from the decimator.
     * if (needsInput), then next call must be acceptData.
     */
    float clock(bool& needsInput)
    {
        --phaseAccumulator;     // one more sample
        if (phaseAccumulator <= 0) {
            needsInput = true;
            phaseAccumulator += rate;
        } else {
            needsInput = false;
        }
        return memory;
    }

    void acceptData(float data)
    {
        memory = data;
    }

    /**
     * Rate must be > 1.
     * Fractional rates are fine.
     */
    void setDecimationRate(float r)
    {
        rate = r;
        phaseAccumulator = rate;
    }

private:
    float rate=0;
    float memory=0;
    float phaseAccumulator = 0;
};
