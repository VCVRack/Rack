#pragma once

namespace rack_plugin_TheXOR {

struct z8kSequencer
{
public:
	void Init(Input *pRst, Input *pDir, Input *pClk, Output *pOut, Light *pLights, std::vector<Param> &params, std::vector<int> steps)
	{
		curStep = 0;
		pReset = pRst;
		pDirection = pDir;
		pClock = pClk;
		pOutput = pOut;
		numSteps = steps.size();
		for(int k = 0; k < numSteps; k++)
		{
			sequence.push_back(&params[steps[k]]);
			leds.push_back(&pLights[steps[k]]);
			chain.push_back(steps[k]);
		}
	}

	int Step()
	{
		if(resetTrigger.process(pReset->value))
			curStep = 0;
		else if(clockTrigger.process(pClock->value))
		{
			if(pDirection->value > 5)
			{
				if(--curStep < 0)
					curStep = numSteps - 1;
			} else
			{
				if(++curStep >= numSteps)
					curStep = 0;
			}
		}

		float v = rescale(sequence[curStep]->value, 0.0, 1.0, 0.0, 6.0);
		pOutput->value = v;
		for(int k = 0; k < numSteps; k++)
			leds[k]->value = k == curStep ? 10.0 : 0;

		return chain[curStep];
	}

private:
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	Input *pReset;
	Input *pDirection;
	Input *pClock;
	Output *pOutput;
	std::vector<Param *> sequence;
	std::vector<Light *> leds;
	std::vector<int> chain;
	int curStep;
	int numSteps;
};

} // namespace rack_plugin_TheXOR
