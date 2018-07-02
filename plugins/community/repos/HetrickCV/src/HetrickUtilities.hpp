#pragma once

#include "dsp/digital.hpp"
#include "engine.hpp"

float LERP(const float _amount, const float _inA, const float _inB);

struct TriggerGenerator
{
    float time = 0.0;
	float triggerTime = 0.001;
    bool process() 
    {
		time += rack::engineGetSampleTime();
		return time < triggerTime;
	}
    void trigger() 
    {
		// Keep the previous pulseTime if the existing pulse would be held longer than the currently requested one.
        if (time + triggerTime >= triggerTime) 
        {
			time = 0.0;
		}
	}
};

struct TriggerGenWithSchmitt
{
	TriggerGenerator trigGen;
	rack::SchmittTrigger schmitt;

	bool process(bool _trigger)
	{
		if(schmitt.process(_trigger ? 2.0f : 0.0f)) trigGen.trigger();
		return trigGen.process();
	}
};

struct HysteresisGate
{
	bool state = false;
	float trueBound = 1.0f;
	float falseBound = 0.98f;

	bool process(float input)
	{
		if(input > trueBound) state = true;
		else if(input < falseBound) state = false;
		return state;
	}
	bool getState() {return state;}
};