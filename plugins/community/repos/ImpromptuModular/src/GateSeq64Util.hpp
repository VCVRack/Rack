//***********************************************************************************************
//Impromptu Modular: Modules for VCV Rack by Marc Boulé
//***********************************************************************************************

#ifndef GATE_SEQ_64_UTIL_HPP
#define GATE_SEQ_64_UTIL_HPP


#include "ImpromptuModular.hpp"
#include "PhraseSeqUtil.hpp"

namespace rack_plugin_ImpromptuModular {

class StepAttributesGS {
	unsigned short attributes;
	
	public:
	
	static const unsigned short ATT_MSK_PROB = 0xFF;
	static const unsigned short ATT_MSK_GATEP = 0x100;
	static const unsigned short ATT_MSK_GATE = 0x200;
	static const unsigned short ATT_MSK_GATEMODE = 0x1C00, gateModeShift = 10;// 3 bits

	static const unsigned short ATT_MSK_INITSTATE =  50;
	
	inline void init() {attributes = ATT_MSK_INITSTATE;}
	inline void randomize() {attributes = ( (randomu32() % 101) | (randomu32() & (ATT_MSK_GATEP | ATT_MSK_GATE | ATT_MSK_GATEMODE)) );}
		
	inline bool getGate() {return (attributes & ATT_MSK_GATE) != 0;}
	inline bool getGateP() {return (attributes & ATT_MSK_GATEP) != 0;}
	inline int getGatePVal() {return attributes & ATT_MSK_PROB;}
	inline int getGateMode() {return (attributes & ATT_MSK_GATEMODE) >> gateModeShift;}
	inline unsigned short getAttribute() {return attributes;}

	inline void setGate(bool gateState) {attributes &= ~ATT_MSK_GATE; if (gateState) attributes |= ATT_MSK_GATE;}
	inline void setGateP(bool gatePState) {attributes &= ~ATT_MSK_GATEP; if (gatePState) attributes |= ATT_MSK_GATEP;}
	inline void setGatePVal(int pVal) {attributes &= ~ATT_MSK_PROB; attributes |= (pVal & ATT_MSK_PROB);}
	inline void setGateMode(int gateMode) {attributes &= ~ATT_MSK_GATEMODE; attributes |= (gateMode << gateModeShift);}
	inline void setAttribute(unsigned short _attributes) {attributes = _attributes;}

	inline void toggleGate() {attributes ^= ATT_MSK_GATE;}
};// class StepAttributesGS 



//*****************************************************************************


class SeqAttributesGS {
	unsigned short attributes;
	
	public:

	static const unsigned short SEQ_MSK_LENGTH  =   0x000000FF;// number of steps in each sequence, min value is 1
	static const unsigned short SEQ_MSK_RUNMODE =   0x0000FF00, runModeShift = 8;
	
	inline void init(int length, int runMode) {attributes = ((length) | (((unsigned short)runMode) << runModeShift));}
	inline void randomize(int maxSteps, int numModes) {attributes = ( (1 + (randomu32() % maxSteps)) | (((unsigned short)(randomu32() % numModes) << runModeShift)) );}
	
	inline int getLength() {return (int)(attributes & SEQ_MSK_LENGTH);}
	inline int getRunMode() {return (int)((attributes & SEQ_MSK_RUNMODE) >> runModeShift);}
	inline unsigned short getSeqAttrib() {return attributes;}
	
	inline void setLength(int length) {attributes &= ~SEQ_MSK_LENGTH; attributes |= ((unsigned short)length);}
	inline void setRunMode(int runMode) {attributes &= ~SEQ_MSK_RUNMODE; attributes |= (((unsigned short)runMode) << runModeShift);}
	inline void setSeqAttrib(unsigned short _attributes) {attributes = _attributes;}
};// class SeqAttributesGS


//*****************************************************************************


inline int ppsToIndexGS(int pulsesPerStep) {// map 1,4,6,12,24, to 0,1,2,3,4
	if (pulsesPerStep == 1) return 0;
	if (pulsesPerStep == 4) return 1; 
	if (pulsesPerStep == 6) return 2;
	if (pulsesPerStep == 12) return 3; 
	return 4; 
}
inline int indexToPpsGS(int index) {// inverse map of ppsToIndex()
	index = clamp(index, 0, 4); 
	if (index == 0) return 1;
	if (index == 1) return 4; 
	if (index == 2) return 6;
	if (index == 3) return 12; 
	return 24; 
}

inline bool calcGate(int gateCode, Trigger clockTrigger) {
	if (gateCode < 2) 
		return gateCode == 1;
	return clockTrigger.isHigh();
}		

} // namespace rack_plugin_ImpromptuModular

#endif
