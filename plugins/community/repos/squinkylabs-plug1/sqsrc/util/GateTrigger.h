#pragma once

#include "SchmidtTrigger.h"
#include <assert.h>


/**
 * Gate trigger if for processing gate and trigger inputs.
 * Features:
 *      SchmidtTrigger on the input, to condition it.
 *      Level sensor, for gate inputs.
 *      Edge detector, for trigger inputs.
 */
class GateTrigger
{
public:
    GateTrigger(bool wantResetLogic) :
        _gate(false),
        _trigger(false),
        _reset(wantResetLogic)
    {
    }

    /**
     * Clock in one input sample. Afterwards may query
     * gate() and trigger()
     */
    void go(float v)
    {
        const bool newGate = _sc.go(v);
        if (_reset) {
            if (newGate)		// in reset state need to wait for low
                return;
            else
                _reset = false;
        }
        _trigger = newGate && !_gate;
        _gate = newGate;
    }

    void reset()
    {
        _gate = false;
        _trigger = false;
        _reset = true;
    }

    bool gate() const
    {
        return _gate;
    }

    bool trigger() const
    {
        return _trigger;
    }

    float thhi() const
    {
        return _sc.thhi();
    }

    float thlo() const
    {
        return _sc.thlo();
    }

private:
    SchmidtTrigger _sc;
    bool _gate;
    bool _trigger;
    bool _reset;		// just reset - gate must go low before high to trigger
};



