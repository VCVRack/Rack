#pragma once

template<typename, int>
class BiquadParams;

template<typename T>
class HilbertFilterDesigner
{
public:
    HilbertFilterDesigner() = delete;       // we are only static
    /**
     * generates a pair of biquads, on will be 90 degrees shifter from the other
     */
    static void design(double sampleRate, BiquadParams<T, 3>& pOutSin, BiquadParams<T, 3>& pOutCos);
};
