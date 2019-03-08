//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************


#include "PhraseSeqUtil.hpp"

namespace rack_plugin_ImpromptuModular {
static const uint32_t advGateHitMask[NUM_GATES] = 
{0x00003F, 0x0F0F0F, 0x000FFF, 0x0F0F00, 0x03FFFF, 0xFFFFFF, 0x00000F, 0x03F03F, 0x000F00, 0x03F000, 0x0F0000, 0};
//	  25%		TRI		  50%		T23		  75%		FUL		  TR1 		DUO		  TR2 	     D2		  TR3  TRIG		


int getAdvGate(int ppqnCount, int pulsesPerStep, int gateMode) { 
	if (gateMode == 11)
		return ppqnCount == 0 ? 3 : 0;
	uint32_t shiftAmt = ppqnCount * (24 / pulsesPerStep);
	return (int)((advGateHitMask[gateMode] >> shiftAmt) & (uint32_t)0x1);
}

int calcGate1Code(StepAttributes attribute, int ppqnCount, int pulsesPerStep, float randKnob) {
	// -1 = gate off for whole step, 0 = gate off for current ppqn, 1 = gate on, 2 = clock high, 3 = trigger
	if (ppqnCount == 0 && attribute.getGate1P() && !(randomUniform() < randKnob))// randomUniform is [0.0, 1.0), see include/util/common.hpp
		return -1;// must do this first in this method since it will kill rest of step if prob turns off the step
	if (!attribute.getGate1())
		return 0;
	int gateType = attribute.getGate1Mode();
	if (pulsesPerStep == 1 && gateType == 0)
		return 2;// clock high
	if (gateType == 11)
		return (ppqnCount == 0 ? 3 : 0);
	return getAdvGate(ppqnCount, pulsesPerStep, gateType);
}

int calcGate2Code(StepAttributes attribute, int ppqnCount, int pulsesPerStep) {
	// 0 = gate off, 1 = clock high, 2 = trigger, 3 = gate on
	if (!attribute.getGate2())
		return 0;
	int gateType = attribute.getGate2Mode();
	if (pulsesPerStep == 1 && gateType == 0)
		return 2;// clock high
	if (gateType == 11)
		return (ppqnCount == 0 ? 3 : 0);
	return getAdvGate(ppqnCount, pulsesPerStep, gateType);
}

bool moveIndexRunMode(int* index, int numSteps, int runMode, unsigned long* history) {// some of this code if from PS32EX)
	int reps = 1;
	// assert((reps * numSteps) <= 0xFFF); // for BRN and RND run modes, history is not a span count but a step count
	
	bool crossBoundary = false;
	
	switch (runMode) {
	
		// history 0x0000 is reserved for reset
		
		case MODE_REV :// reverse; history base is 0x2000
			if ((*history) < 0x2001 || (*history) > 0x2FFF)
				(*history) = 0x2000 + reps;
			(*index)--;
			if ((*index) < 0) {
				(*index) = numSteps - 1;
				(*history)--;
				if ((*history) <= 0x2000)
					crossBoundary = true;
			}
		break;
		
		case MODE_PPG :// forward-reverse; history base is 0x3000
			if ((*history) < 0x3001 || (*history) > 0x3FFF) // even means going forward, odd means going reverse
				(*history) = 0x3000 + reps * 2;
			if (((*history) & 0x1) == 0) {// even so forward phase
				(*index)++;
				if ((*index) >= numSteps) {
					(*index) = numSteps - 1 ;
					(*history)--;
				}
			}
			else {// odd so reverse phase
				(*index)--;
				if ((*index) < 0) {
					(*index) = 0;
					(*history)--;
					if ((*history) <= 0x3000)
						crossBoundary = true;
				}
			}
		break;

		case MODE_PEN :// forward-reverse; history base is 0x4000
			if ((*history) < 0x4001 || (*history) > 0x4FFF) // even means going forward, odd means going reverse
				(*history) = 0x4000 + reps * 2;
			if (((*history) & 0x1) == 0) {// even so forward phase
				(*index)++;
				if ((*index) >= numSteps) {
					(*index) = numSteps - 2;
					(*history)--;
					if ((*index) < 1) {// if back at 0 after turnaround, then no reverse phase needed
						(*index) = 0;
						(*history)--;
						if ((*history) <= 0x4000)
							crossBoundary = true;
					}
				}
			}
			else {// odd so reverse phase
				(*index)--;
				if ((*index) < 1) {
					(*index) = 0;
					(*history)--;
					if ((*history) <= 0x4000)
						crossBoundary = true;
				}
			}
		break;
		
		case MODE_BRN :// brownian random; history base is 0x5000
			if ((*history) < 0x5001 || (*history) > 0x5FFF) 
				(*history) = 0x5000 + numSteps * reps;
			(*index) += (randomu32() % 3) - 1;
			if ((*index) >= numSteps) {
				(*index) = 0;
			}
			if ((*index) < 0) {
				(*index) = numSteps - 1;
			}
			(*history)--;
			if ((*history) <= 0x5000) {
				crossBoundary = true;
			}
		break;
		
		case MODE_RND :// random; history base is 0x6000
		case MODE_RN2 :
			if ((*history) < 0x6001 || (*history) > 0x6FFF) 
				(*history) = 0x6000 + numSteps * reps;
			(*index) = (randomu32() % numSteps) ;
			(*history)--;
			if ((*history) <= 0x6000) {
				crossBoundary = true;
			}
		break;
		
		//case MODE_FW2 :// forward twice
		//case MODE_FW3 :// forward three times
		//case MODE_FW4 :// forward four times
		default :// MODE_FWD  forward; history base is 0x1000
			if (runMode == MODE_FW2) reps++;
			else if (runMode == MODE_FW3) reps += 2;
			else if (runMode == MODE_FW4) reps += 3;
			if ((*history) < 0x1001 || (*history) > 0x1FFF)
				(*history) = 0x1000 + reps;
			(*index)++;
			if ((*index) >= numSteps) {
				(*index) = 0;
				(*history)--;
				if ((*history) <= 0x1000)
					crossBoundary = true;
			}
	}

	return crossBoundary;
}


int keyIndexToGateMode(int keyIndex, int pulsesPerStep) {
	int ret = keyIndex;
	
	if (keyIndex == 1 || keyIndex == 3 || keyIndex == 6 || keyIndex == 8 || keyIndex == 10) {// black keys
		if ((pulsesPerStep % 6) != 0)
			ret = -1;
	}
	else if (keyIndex == 4 || keyIndex == 7 || keyIndex == 9) {// 75%, DUO, DU2 
		if ((pulsesPerStep % 4) != 0)
			ret = -1;
	}
	else if (keyIndex == 2) {// 50%
		if ((pulsesPerStep % 2) != 0)
			ret = -1;
	}
	else if (keyIndex == 0) {// 25%
		if (pulsesPerStep != 1 && (pulsesPerStep % 4) != 0)
			ret = -1;
	}
	//else always good: 5 (full) and 11 (trig)
	
	return ret;
}

} // namespace rack_plugin_ImpromptuModular

/*CHANGE LOG

0.6.12:
revert PPG and add the new one as a run mode called PND (Pendulum); fix PS, SMS, GS toJson/fromJson to adjust old patches
fix PPG run mode, so that it is a true PPG (ex: 1,2,3,2,1,2... instead of 1,2,3,3,2,1,1,2...)

*/
