//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************

#include "dsp/digital.hpp"

using namespace rack;

namespace rack_plugin_ImpromptuModular {

// General constants

enum RunModeIds {MODE_FWD, MODE_REV, MODE_PPG, MODE_BRN, MODE_RND, MODE_FW2, MODE_FW3, MODE_FW4, NUM_MODES};
static const std::string modeLabels[NUM_MODES]={"FWD","REV","PPG","BRN","RND","FW2","FW3","FW4"};

static const int NUM_GATES = 12;												
static const uint32_t advGateHitMask[NUM_GATES] = 
{0x00003F, 0x0F0F0F, 0x000FFF, 0x0F0F00, 0x03FFFF, 0xFFFFFF, 0x00000F, 0x03F03F, 0x000F00, 0x03F000, 0x0F0000, 0};
//	  25%		TRI		  50%		T23		  75%		FUL		  TR1 		DUO		  TR2 	     D2		  TR3  TRIG		

enum AttributeBitMasks {ATT_MSK_GATE1 = 0x01, ATT_MSK_GATE1P = 0x02, ATT_MSK_GATE2 = 0x04, ATT_MSK_SLIDE = 0x08, ATT_MSK_TIED = 0x10};// 5 bits
static const int ATT_MSK_GATE1MODE = 0x01E0;// 4 bits
static const int gate1ModeShift = 5;
static const int ATT_MSK_GATE2MODE = 0x1E00;// 4 bits
static const int gate2ModeShift = 9;

							
				
// Inline methods
inline bool getGate1a(int attribute) {return (attribute & ATT_MSK_GATE1) != 0;}
inline bool getGate1Pa(int attribute) {return (attribute & ATT_MSK_GATE1P) != 0;}
inline bool getGate2a(int attribute) {return (attribute & ATT_MSK_GATE2) != 0;}
inline bool getSlideA(int attribute) {return (attribute & ATT_MSK_SLIDE) != 0;}
inline bool getTiedA(int attribute) {return (attribute & ATT_MSK_TIED) != 0;}
inline int getGate1aMode(int attribute) {return (attribute & ATT_MSK_GATE1MODE) >> gate1ModeShift;}
inline int getGate2aMode(int attribute) {return (attribute & ATT_MSK_GATE2MODE) >> gate2ModeShift;}

inline void setGate1a(int *attribute, bool gate1State) {(*attribute) &= ~ATT_MSK_GATE1; if (gate1State) (*attribute) |= ATT_MSK_GATE1;}
inline void setGate1Pa(int *attribute, bool gate1PState) {(*attribute) &= ~ATT_MSK_GATE1P; if (gate1PState) (*attribute) |= ATT_MSK_GATE1P;}
inline void setGate2a(int *attribute, bool gate2State) {(*attribute) &= ~ATT_MSK_GATE2; if (gate2State) (*attribute) |= ATT_MSK_GATE2;}
inline void setSlideA(int *attribute, bool slideState) {(*attribute) &= ~ATT_MSK_SLIDE; if (slideState) (*attribute) |= ATT_MSK_SLIDE;}
inline void setTiedA(int *attribute, bool tiedState) {(*attribute) &= ~ATT_MSK_TIED; if (tiedState) (*attribute) |= ATT_MSK_TIED;}

inline void toggleGate1a(int *attribute) {(*attribute) ^= ATT_MSK_GATE1;}
inline void toggleGate1Pa(int *attribute) {(*attribute) ^= ATT_MSK_GATE1P;}
inline void toggleGate2a(int *attribute) {(*attribute) ^= ATT_MSK_GATE2;}
inline void toggleSlideA(int *attribute) {(*attribute) ^= ATT_MSK_SLIDE;}
inline void toggleTiedA(int *attribute) {(*attribute) ^= ATT_MSK_TIED;}


inline int ppsToIndex(int pulsesPerStep) {// map 1,4,6,12,24, to 0,1,2,3,4
	if (pulsesPerStep == 1) return 0;
	if (pulsesPerStep == 4) return 1; 
	if (pulsesPerStep == 6) return 2;
	if (pulsesPerStep == 12) return 3; 
	return 4; 
}
inline int indexToPps(int index) {// inverse map of ppsToIndex()
	index = clamp(index, 0, 4); 
	if (index == 0) return 1;
	if (index == 1) return 4; 
	if (index == 2) return 6;
	if (index == 3) return 12; 
	return 24; 
}

inline bool calcGate(int gateCode, SchmittTrigger clockTrigger, unsigned long clockStep, float sampleRate) {
	if (gateCode < 2) 
		return gateCode == 1;
	if (gateCode == 2)
		return clockTrigger.isHigh();
	return clockStep < (unsigned long) (sampleRate * 0.001f);
}

inline int getAdvGate(int ppqnCount, int pulsesPerStep, int gateMode) { 
	if (gateMode == 11)
		return ppqnCount == 0 ? 3 : 0;
	uint32_t shiftAmt = ppqnCount * (24 / pulsesPerStep);
	return (int)((advGateHitMask[gateMode] >> shiftAmt) & (uint32_t)0x1);
}

inline int calcGate1Code(int attribute, int ppqnCount, int pulsesPerStep, float randKnob) {
	// -1 = gate off for whole step, 0 = gate off for current ppqn, 1 = gate on, 2 = clock high, 3 = trigger
	if (ppqnCount == 0 && getGate1Pa(attribute) && !(randomUniform() < randKnob))// randomUniform is [0.0, 1.0), see include/util/common.hpp
		return -1;// must do this first in this method since it will kill rest of step if prob turns off the step
	if (!getGate1a(attribute))
		return 0;
	if (pulsesPerStep == 1)
		return 2;// clock high
	return getAdvGate(ppqnCount, pulsesPerStep, getGate1aMode(attribute));
}
inline int calcGate2Code(int attribute, int ppqnCount, int pulsesPerStep) {
	// 0 = gate off, 1 = clock high, 2 = trigger, 3 = gate on
	if (!getGate2a(attribute))
		return 0;
	if (pulsesPerStep == 1)
		return 2;// clock high
	return getAdvGate(ppqnCount, pulsesPerStep, getGate2aMode(attribute));
}

inline int gateModeToKeyLightIndex(int attribute, bool isGate1) {// keyLight index now matches gate modes, so no mapping table needed anymore
	return isGate1 ? getGate1aMode(attribute) : getGate2aMode(attribute);
}



// Other methods (code in PhraseSeqUtil.cpp)	
												
int moveIndex(int index, int indexNext, int numSteps);
bool moveIndexRunMode(int* index, int numSteps, int runMode, int* history);
int keyIndexToGateMode(int keyIndex, int pulsesPerStep);

} // namespace rack_plugin_ImpromptuModular
