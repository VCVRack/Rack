#pragma once

namespace rack_plugin_TheXOR {

struct Spiralone;
struct spiraloneSequencer
{
public:
	void Reset(int seq, Spiralone *pSpir);
	void Step(int seq, Spiralone *pSpir);

private:
	SchmittTrigger2 clockTrig;
	SchmittTrigger resetTrigger;
	int curPos;

	int ledID(int seq) { return ledID(seq, curPos); }
	int ledID(int seq, int n) { return seq * TOTAL_STEPS + n; }
	int getInput(int seq, Spiralone *pSpir, int input_id, int knob_id, float minValue, float maxValue);
	void gate(int clk, int seq, Spiralone *pSpir);
	void outputVoltage(int seq, Spiralone *pSpir);
};

} // namespace rack_plugin_TheXOR
