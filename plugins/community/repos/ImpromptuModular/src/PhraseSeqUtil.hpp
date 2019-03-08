//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************

#ifndef PHRASE_SEQ_UTIL_HPP
#define PHRASE_SEQ_UTIL_HPP

#include "ImpromptuModular.hpp"

namespace rack_plugin_ImpromptuModular {

// General constants

enum RunModeIds {MODE_FWD, MODE_REV, MODE_PPG, MODE_PEN, MODE_BRN, MODE_RND, MODE_FW2, MODE_FW3, MODE_FW4, MODE_RN2, NUM_MODES};
static const std::string modeLabels[NUM_MODES] = {"FWD","REV","PPG","PEN","BRN","RND","FW2","FW3","FW4","RN2"};// PS16 and SMS16 use NUM_MODES - 1 since no RN2!!!

static const int NUM_GATES = 12;// advanced gate types												


//*****************************************************************************


class StepAttributes {
	unsigned short attributes;
	
	public:

	static const unsigned short ATT_MSK_GATE1 = 0x01;
	static const unsigned short ATT_MSK_GATE1P = 0x02;
	static const unsigned short ATT_MSK_GATE2 = 0x04;
	static const unsigned short ATT_MSK_SLIDE = 0x08;
	static const unsigned short ATT_MSK_TIED = 0x10;
	static const unsigned short ATT_MSK_GATE1MODE = 0x01E0, gate1ModeShift = 5;
	static const unsigned short ATT_MSK_GATE2MODE = 0x1E00, gate2ModeShift = 9;
	
	static const unsigned short ATT_MSK_INITSTATE =  ATT_MSK_GATE1;
	
	inline void clear() {attributes = 0u;}
	inline void init() {attributes = ATT_MSK_INITSTATE;}
	inline void randomize() {attributes = (randomu32() & (ATT_MSK_GATE1 | ATT_MSK_GATE1P | ATT_MSK_GATE2 | ATT_MSK_SLIDE /*| ATT_MSK_TIED*/ | ATT_MSK_GATE1MODE | ATT_MSK_GATE2MODE));}
	
	inline bool getGate1() {return (attributes & ATT_MSK_GATE1) != 0;}
	inline bool getGate1P() {return (attributes & ATT_MSK_GATE1P) != 0;}
	inline bool getGate2() {return (attributes & ATT_MSK_GATE2) != 0;}
	inline bool getSlide() {return (attributes & ATT_MSK_SLIDE) != 0;}
	inline bool getTied() {return (attributes & ATT_MSK_TIED) != 0;}
	inline int getGate1Mode() {return (attributes & ATT_MSK_GATE1MODE) >> gate1ModeShift;}
	inline int getGate2Mode() {return (attributes & ATT_MSK_GATE2MODE) >> gate2ModeShift;}
	inline unsigned short getAttribute() {return attributes;}

	inline void setGate1(bool gate1State) {attributes &= ~ATT_MSK_GATE1; if (gate1State) attributes |= ATT_MSK_GATE1;}
	inline void setGate1P(bool gate1PState) {attributes &= ~ATT_MSK_GATE1P; if (gate1PState) attributes |= ATT_MSK_GATE1P;}
	inline void setGate2(bool gate2State) {attributes &= ~ATT_MSK_GATE2; if (gate2State) attributes |= ATT_MSK_GATE2;}
	inline void setSlide(bool slideState) {attributes &= ~ATT_MSK_SLIDE; if (slideState) attributes |= ATT_MSK_SLIDE;}
	inline void setTied(bool tiedState) {
		attributes &= ~ATT_MSK_TIED; 
		if (tiedState) {
			attributes |= ATT_MSK_TIED;
			attributes &= ~(ATT_MSK_GATE1 | ATT_MSK_GATE1P | ATT_MSK_GATE2 | ATT_MSK_SLIDE);// clear other attributes if tied
		}
	}
	inline void setGate1Mode(int gateMode) {attributes &= ~ATT_MSK_GATE1MODE; attributes |= (gateMode << gate1ModeShift);}
	inline void setGate2Mode(int gateMode) {attributes &= ~ATT_MSK_GATE2MODE; attributes |= (gateMode << gate2ModeShift);}
	inline void setGateMode(int gateMode, bool gate1) {if (gate1) setGate1Mode(gateMode); else setGate2Mode(gateMode);}
	inline void setAttribute(unsigned short _attributes) {attributes = _attributes;}

	inline void toggleGate1() {attributes ^= ATT_MSK_GATE1;}
	inline void toggleGate1P() {attributes ^= ATT_MSK_GATE1P;}
	inline void toggleGate2() {attributes ^= ATT_MSK_GATE2;}
	inline void toggleSlide() {attributes ^= ATT_MSK_SLIDE;}
};// class StepAttributes



//*****************************************************************************


class SeqAttributes {
	unsigned long attributes;
	
	public:

	static const unsigned long SEQ_MSK_LENGTH  =   0x000000FF;// number of steps in each sequence, min value is 1
	static const unsigned long SEQ_MSK_RUNMODE =   0x0000FF00, runModeShift = 8;
	static const unsigned long SEQ_MSK_TRANSPOSE = 0x007F0000, transposeShift = 16;
	static const unsigned long SEQ_MSK_TRANSIGN =  0x00800000;// manually implement sign bit
	static const unsigned long SEQ_MSK_ROTATE =    0x7F000000, rotateShift = 24;
	static const unsigned long SEQ_MSK_ROTSIGN =   0x80000000;// manually implement sign bit (+ is right, - is left)
	
	inline void init(int length, int runMode) {attributes = ((length) | (((unsigned long)runMode) << runModeShift));}
	inline void randomize(int maxSteps, int numModes) {attributes = ( (1 + (randomu32() % maxSteps)) | (((unsigned long)(randomu32() % numModes) << runModeShift)) );}
	
	inline int getLength() {return (int)(attributes & SEQ_MSK_LENGTH);}
	inline int getRunMode() {return (int)((attributes & SEQ_MSK_RUNMODE) >> runModeShift);}
	inline int getTranspose() {
		int ret = (int)((attributes & SEQ_MSK_TRANSPOSE) >> transposeShift);
		if ( (attributes & SEQ_MSK_TRANSIGN) != 0)// if negative
			ret *= -1;
		return ret;
	}
	inline int getRotate() {
		int ret = (int)((attributes & SEQ_MSK_ROTATE) >> rotateShift);
		if ( (attributes & SEQ_MSK_ROTSIGN) != 0)// if negative
			ret *= -1;
		return ret;
	}
	inline unsigned long getSeqAttrib() {return attributes;}
	
	inline void setLength(int length) {attributes &= ~SEQ_MSK_LENGTH; attributes |= ((unsigned long)length);}
	inline void setRunMode(int runMode) {attributes &= ~SEQ_MSK_RUNMODE; attributes |= (((unsigned long)runMode) << runModeShift);}
	inline void setTranspose(int transp) {
		attributes &= ~ (SEQ_MSK_TRANSPOSE | SEQ_MSK_TRANSIGN); 
		attributes |= (((unsigned long)abs(transp)) << transposeShift);
		if (transp < 0) 
			attributes |= SEQ_MSK_TRANSIGN;
	}
	inline void setRotate(int rotn) {
		attributes &= ~ (SEQ_MSK_ROTATE | SEQ_MSK_ROTSIGN); 
		attributes |= (((unsigned long)abs(rotn)) << rotateShift);
		if (rotn < 0) 
			attributes |= SEQ_MSK_ROTSIGN;
	}
	inline void setSeqAttrib(unsigned long _attributes) {attributes = _attributes;}
};// class SeqAttributes


//*****************************************************************************
		

inline int ppsToIndex(int pulsesPerStep) {// map 1,2,4,6,8,10,12...24, to 0,1,2,3,4,5,6...12
	if (pulsesPerStep == 1) return 0;
	return pulsesPerStep >> 1;
}
inline int indexToPps(int index) {// inverse map of ppsToIndex()
	index = clamp(index, 0, 12);
	if (index == 0) return 1;
	return index <<	1;
}

inline float applyNewOct(float cvVal, int newOct) {
	float newCV = cvVal + 10.0f;//to properly handle negative note voltages
	return newCV - floor(newCV) + (float) (newOct - 3);
}

inline bool calcGate(int gateCode, Trigger clockTrigger, unsigned long clockStep, float sampleRate) {
	if (gateCode < 2) 
		return gateCode == 1;
	if (gateCode == 2)
		return clockTrigger.isHigh();
	return clockStep < (unsigned long) (sampleRate * 0.01f);
}

inline int gateModeToKeyLightIndex(StepAttributes attribute, bool isGate1) {// keyLight index now matches gate modes, so no mapping table needed anymore
	return isGate1 ? attribute.getGate1Mode() : attribute.getGate2Mode();
}



// Other methods (code in PhraseSeqUtil.cpp)	

int getAdvGate(int ppqnCount, int pulsesPerStep, int gateMode);
int calcGate1Code(StepAttributes attribute, int ppqnCount, int pulsesPerStep, float randKnob);
int calcGate2Code(StepAttributes attribute, int ppqnCount, int pulsesPerStep);
bool moveIndexRunMode(int* index, int numSteps, int runMode, unsigned long* history);
int keyIndexToGateMode(int keyIndex, int pulsesPerStep);


} // namespace rack_plugin_ImpromptuModular

#endif
