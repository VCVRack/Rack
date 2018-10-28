
#pragma once

template<typename, int>
class BiquadParams;

template <typename T>
class ButterworthFilterDesigner
{
public:
    ButterworthFilterDesigner() = delete;       // we are only static
    static void designEightPoleLowpass(BiquadParams<T, 4>& pOut, T frequency);
    static void designSixPoleLowpass(BiquadParams<T, 3>& pOut, T frequency);
    static void designThreePoleLowpass(BiquadParams<T, 2>& pOut, T frequency);
    static void designFourPoleLowpass(BiquadParams<T, 2>& pOut, T frequency);
    static void designFourPoleHighpass(BiquadParams<T, 2>& pOut, T frequency);
    static void designFivePoleLowpass(BiquadParams<T, 3>& pOut, T frequency);
    static void designTwoPoleLowpass(BiquadParams<T, 1>& pOut, T frequency);

    static void designSixPoleElliptic(BiquadParams<T, 3>& pOut, T frequency, T rippleDb, T stopbandAttenDb);
    static void designEightPoleElliptic(BiquadParams<T, 4>& pOut, T frequency, T rippleDb, T stopbandAttenDb);
};