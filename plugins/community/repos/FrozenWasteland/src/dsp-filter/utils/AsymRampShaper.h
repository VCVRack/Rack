#pragma once

#include <assert.h>


/** For Tremolo: lets you smoothly adjust phase and skew
* turns a ramp into a continuously variable dual slope ramp:
*		at one extreme it is increasing ramp
*		at other extreme it is decreasing
*		in between it it triangle
*/

class AsymRampShaperParams
{
public:
    float	k;		//when input < k we are in the rising phase
    float a1;		// in rising phase, f(x) = a1 * x;
    float a2;
    float b2;		// in falling phase, f(x) = a2 * x + b2
    float phaseOffset;	// between 0 and 1

    bool valid()
    {
        bool ret = true;
        ret &= (k > 0 && k < 1);
        ret &= (phaseOffset >= 0 && phaseOffset <= 1);
        return ret;
    }
};


class AsymRampShaper
{
public:

    /* skew = -1..1, zero is triangle
    * phase = -1..1, zero is no shift
    */
    static void setup(AsymRampShaperParams& params, float skew, float phase)
    {
        // first deal with phase
        if (phase < 0) phase += 1;		// adjust phase into range convenient for us 
                                        // (it all wraps around two pi anyway)

        assert(phase >= 0 && phase <= 1);
        params.phaseOffset = phase;

        // now skew
        assert(skew > -1 && skew < 1);
        params.k = (skew + 1) / (float) 2;		// between 0 and 1

        params.a1 = float(1.0 / params.k);					// as x goes 0..k, f(x) 0..1
        params.a2 = float(1.0 / (params.k - 1.0));
        params.b2 = -params.a2;

        assert(params.valid());
        //char buf[512];
        //sprintf(buf, "set k=%f a1=%f a2=%f b2=%f\n", params.k, params.a1, params.a2, params.b2);
        //DebugUtil::trace(buf);
    }

    static float proc_1(AsymRampShaperParams& params, float input)
    {
        assert(params.valid());
        input += params.phaseOffset;
        if (input > 1) input -= 1;

        float ret = 0;
        if (input < params.k) {
            ret = input * params.a1;
        } else {
            ret = params.b2 + input * params.a2;
        }
        assert(ret >= 0 && ret <= 1);
        return ret;
    }

};


