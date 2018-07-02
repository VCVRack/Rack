#pragma once
#include "InterpDelay.hpp"

template<class T>
class AllpassFilter {
public:
    AllpassFilter() {
        clear();
        gain = 0;
    }

    AllpassFilter(long maxDelay, long initDelay, T gain) {
        clear();
        delay = InterpDelay<T>(maxDelay, initDelay);
        this->gain = gain;
    }

    T inline process() {
        _inSum = input + delay.output * gain;
        output = delay.output + _inSum * gain * -1;
        delay.input = _inSum;
        delay.process();
        return output;
    }

    void clear() {
        input = 0;
        output = 0;
        _inSum = 0;
        _outSum = 0;
        delay.clear();
    }

    T input;
    T gain;
    T output;
    InterpDelay<T> delay;
private:
    T _inSum;
    T _outSum;
};
