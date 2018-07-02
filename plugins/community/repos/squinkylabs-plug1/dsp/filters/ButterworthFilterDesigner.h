
#pragma once

template<typename, int>
class BiquadParams;

template <typename T>
class ButterworthFilterDesigner
{
public:
    ButterworthFilterDesigner() = delete;       // we are only static
    static void designThreePoleLowpass(BiquadParams<T, 2>& pOut, T frequency);
    static void designTwoPoleLowpass(BiquadParams<T, 1>& pOut, T frequency);
};