#pragma once

#include "GateTrigger.h"

/**
 * Output processing for triggers.
 * Outputs a finite duration trigger when gate changes.
 */
class TriggerOutput
{
public:
    TriggerOutput() :
        _gateProcessor(false),		    // defeat reset logic that we don't want
        _counter(0),
        _duration(TRIGGER_OUT_TIME_MS * 44100 / 1000)             // TODO: make this better
    {
    }
    void go(bool gate)
    {
        if (_counter) {
            --_counter;
            return;
        }
        _gateProcessor.go(gate ? cGateOutHi : cGateOutLow);
        if (_gateProcessor.trigger()) {
            _counter = _duration;
        }

    }
    float get() const
    {
        return (_counter > 0) ? cGateOutHi : cGateOutLow;     // TODO: 0..10 for gates/trig/clock?
    }
private:
    GateTrigger _gateProcessor;
    int _counter;
    const int _duration;
};

