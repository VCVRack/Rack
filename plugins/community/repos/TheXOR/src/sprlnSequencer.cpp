#include "Spiralone.hpp"
#include "sprlnSequencer.hpp"
#include "SpiraloneModule.hpp"

namespace rack_plugin_TheXOR {

extern float AccessParam(Spiralone *p, int seq, int id);
extern float AccessParam(Spiralone *p, int id);
extern Input *AccessInput(Spiralone *p, int seq, int id);
extern float *AccessOutput(Spiralone *p, int seq, int id);
extern float *AccessLight(Spiralone *p, int id);

void spiraloneSequencer::Step(int seq, Spiralone *pSpir)
{
	if(resetTrigger.process(AccessInput(pSpir, seq, Spiralone::RESET_1)->value))
		Reset(seq, pSpir);
	else
	{
		int clk = clockTrig.process(AccessInput(pSpir, seq, Spiralone::CLOCK_1)->value); // 1=rise, -1=fall
		if(clk == 1)
		{
			int mode = (int)std::roundf(AccessParam(pSpir, seq, Spiralone::MODE_1));
			int numSteps = getInput(seq, pSpir, Spiralone::INLENGHT_1, Spiralone::LENGHT_1, 1.0, TOTAL_STEPS);
			int stride = getInput(seq, pSpir, Spiralone::INSTRIDE_1, Spiralone::STRIDE_1, 1.0, 8.0);

			*AccessLight(pSpir, ledID(seq)) = 0.0;
			switch(mode)
			{
			case 0: // fwd:
				curPos += stride;
				break;

			case 1: // bwd
				curPos -= stride;
				break;
			}
			if(curPos < 0)
				curPos = numSteps + curPos;

			curPos %= numSteps;

			outputVoltage(seq, pSpir);
			gate(clk, seq, pSpir);
		} else if(clk == -1)
			gate(clk, seq, pSpir);
	}
}

void spiraloneSequencer::Reset(int seq, Spiralone *pSpir)
{
	curPos = 0;
	for(int k = 0; k < TOTAL_STEPS; k++)
		*AccessLight(pSpir, ledID(seq, k)) = 0.0;
}

int spiraloneSequencer::getInput(int seq, Spiralone *pSpir, int input_id, int knob_id, float minValue, float maxValue)
{
	float normalized_in = AccessInput(pSpir, seq, input_id)->active ? rescale(AccessInput(pSpir, seq, input_id)->value, 0.0, 5.0, 0.0, maxValue) : 0.0;
	float v = clamp(normalized_in + AccessParam(pSpir, seq, knob_id), minValue, maxValue);
	return (int)roundf(v);
}

void spiraloneSequencer::outputVoltage(int seq, Spiralone *pSpir)
{
	float v = AccessParam(pSpir, seq, Spiralone::XPOSE_1);
	if(AccessInput(pSpir, seq, Spiralone::INXPOSE_1)->active)
		v += AccessInput(pSpir, seq, Spiralone::INXPOSE_1)->value;
	v += AccessParam(pSpir, Spiralone::VOLTAGE_1 + curPos);
	*AccessOutput(pSpir, seq, Spiralone::CV_1) = clamp(v, 0.0, 10.0);
}

void spiraloneSequencer::gate(int clk, int seq, Spiralone *pSpir)
{
	if(clk == 1)
	{
		*AccessLight(pSpir, ledID(seq)) = 10.0;
		*AccessOutput(pSpir, seq, Spiralone::GATE_1) = LVL_ON;
	} else if(clk == -1) // fall
	{
		*AccessOutput(pSpir, seq, Spiralone::GATE_1) = LVL_OFF;
	}
}

} // namespace rack_plugin_TheXOR
