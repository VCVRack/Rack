#ifndef MODULE_TRIGSEQ_HPP
#define MODULE_TRIGSEQ_HPP

#include <string.h>
#include <stdio.h>
//#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

#define trigSeq_GATE_ON_OUTPUT	  10.0  // If gate is on, the value to output (port Voltage)
#define trigSeq_GATE_OFF_OUTPUT	   0.0  // If gate is off, the value to output (port Voltage)

// Single instance to the trigSeq Models.
// trigSeq (16-step) model
extern Model* modelTrigSeq;
// trigSeq (64-step) model
extern Model* modelTrigSeq64;


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq Module
// trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq : TSSequencerModuleBase
{	
	// Move these back to base
	//SchmittTrigger* gateTriggers;
	
	trigSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, false)
	{
		gateTriggers = new SchmittTrigger[numSteps]; // maxSteps
		selectedOutputValueMode = VALUE_TRIGGER;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "TRIG";
		modeStrings[1] = "RTRG";
		modeStrings[2] = "GATE";		
		return;
	}
	trigSeq() : trigSeq(TROWA_SEQ_NUM_STEPS, TROWA_SEQ_STEP_NUM_ROWS, TROWA_SEQ_STEP_NUM_ROWS)
	{
		return;
	}	
	~trigSeq()
	{
		delete [] gateTriggers;	
		gateTriggers = NULL;
		return;
	}
	void step() override;
	// Only randomize the current gate/trigger steps.
	void randomize() override;
	// Get the toggle step value
	float getToggleStepValue(int step, float val, int channel, int pattern) override;
	// Calculate a representation of all channels for this step
	float getPlayingStepValue(int step, int pattern) override;
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64 Module
// trowaSoft 64-step pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq64 : trigSeq {
	trigSeq64() : trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS)
	{
		return;
	}
};


#endif