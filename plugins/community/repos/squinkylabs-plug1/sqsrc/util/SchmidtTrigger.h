
#pragma once

#include "Constants.h"

class SchmidtTrigger
{
public:
    SchmidtTrigger(float thLo = cGateLow, float thHi = cGateHi) :
        _thLo(thLo), _thHi(thHi), _lastOut(false)
    {
    }

    bool go(float input)
    {
        if (_lastOut)		// if we were true last time
        {
            if (input < _thLo) {
                _lastOut = false;
            }
        } else {
            if (input > _thHi) {
                _lastOut = true;
            }
        }
        return _lastOut;
    }

    float thhi() const
    {
        return _thHi;
    }
    float thlo() const
    {
        return _thLo;
    }

private:
    const float _thLo;
    const float _thHi;
    bool	_lastOut;
};


