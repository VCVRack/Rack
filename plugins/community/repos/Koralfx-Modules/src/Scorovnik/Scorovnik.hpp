/**
Scorovnik.hpp
Author: Tomek Sosnowski
Contact: koralfx@gmail.com
Date: 1/4/2018

Copyright 2018 Tomasz Sosnowski. Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 1. Redistributions of
source code must retain the above copyright notice, this list of conditions and the following
disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution. 3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from this software without
specific prior written permission.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef KORALFX_SCOROVNIK_HPP
#define KORALFX_SCOROVNIK_HPP

#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

#include "dsp/digital.hpp"
#include <string>

struct Scorovnik : Module {
	enum ParamIds {
		SLIDER_NOTE_LENGTH_PARAM,
		SLIDER_NOTE_PITCH_PARAM		= SLIDER_NOTE_LENGTH_PARAM + 32,
		SWITCH_NOTE_DOT_TRI_PARAM	= SLIDER_NOTE_PITCH_PARAM + 32,
		SWITCH_NOTE_PAUSE_ACC_PARAM	= SWITCH_NOTE_DOT_TRI_PARAM + 32,
		SWITCH_NOTE_STACCATO_PARAM	= SWITCH_NOTE_PAUSE_ACC_PARAM + 32,
		LED_BUTTON_PARAM			= SWITCH_NOTE_STACCATO_PARAM + 32,
		TEMPO_PARAM					= LED_BUTTON_PARAM + 32,
		TRANSPOSE_PARAM,
		SWITCH_MODE_PARAM,
		KNOB_LOOP_NUMBER_PARAM,
		SWITCH_START_PARAM,
		SWITCH_STOP_PARAM,
		SWITCH_RESET_PARAM,
		SWITCH_MONITOR_PARAM,
		SWITCH_UNI_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXT_CLK_INPUT,
		START_INPUT,
		STOP_INPUT = START_INPUT + 4,
		RESET_INPUT = STOP_INPUT + 4,
		CV_TRANSPOSE_INPUT = RESET_INPUT + 4,		
		NUM_INPUTS 
	};
	enum OutputIds {
		CLK4_OUTPUT,
		CLK16_OUTPUT,
		GATE_OUTPUT,
		PITCH_OUTPUT = GATE_OUTPUT + 4,
		ACCENT_OUTPUT = PITCH_OUTPUT + 4,
		LAST_OUTPUT = ACCENT_OUTPUT + 4,	
		NUM_OUTPUTS  = LAST_OUTPUT + 4
	};
	enum LightIds {
		GROUP_LIGHT,
		SELECTED_STEP_LIGHT = GROUP_LIGHT + 8,
		GROUP_LENGTH_LIGHT = SELECTED_STEP_LIGHT + 32,
		NUM_LIGHTS = GROUP_LENGTH_LIGHT + 32
	};


///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    int panelStyle 			= 0;
    NVGcolor colorDisplay 	= nvgRGB(0x56, 0xdc, 0xff);


    int tempoBPM 			= 0;
    int tempoBPMOld 		= 0;

	int globalTranspose 	= 0;
	int globalTransposeOld 	= -1;

	int loopLimit 			= 0;
	int loopLimitOld 		= -1;

    std::string tempo;
    std::string transpose;
    std::string loopNumber;

    int oldMode 			=-1;
    float externalTime 		= 0;

    bool initStep = true;

    std::string stepNumber;
    std::string notePitchDisplay;

    float sliderValue [32];
	float sliderValueOld [32];

	std::string noteNames [12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};

	float clockPhase;
	int clock16;
	int clockBPM;
	int groupActive [4]		= {0,0,0,0};
	int groupPlay [4]		= {0,0,0,0};
	int groupStep [4] 		= {0,0,0,0};

	int mode1Seq  			= 0;
	int mode2SeqA 			= 0;
	int mode2SeqC 			= 16;
	int mode3SeqA 			= 0;
	int mode3SeqB 			= 8;
	int mode3SeqC 			= 16;
	int mode3SeqD 			= 24;


	int mode1SeqCounter  	= 0;
	int mode2SeqCounterA 	= 0;
	int mode2SeqCounterC 	= 0;
	int mode3SeqCounterA 	= 0;
	int mode3SeqCounterB 	= 0;
	int mode3SeqCounterC 	= 0;
	int mode3SeqCounterD 	= 0;

	int mode1SeqEnd  		= 32;
	int mode2SeqEndA 		= 16;
	int mode2SeqEndC 		= 16;
	int mode3SeqEndA 		= 8;
	int mode3SeqEndB 		= 8;
	int mode3SeqEndC 		= 8;
	int mode3SeqEndD 		= 8;

	int mode1LoopCounter  	= 0;
	int mode2LoopCounterA 	= 0;
	int mode2LoopCounterC 	= 0;
	int mode3LoopCounterA 	= 0;
	int mode3LoopCounterB 	= 0;
	int mode3LoopCounterC 	= 0;
	int mode3LoopCounterD 	= 0;

///////////////////////////////////////////////////////////////////////////////
// Schmitt Triggers
///////////////////////////////////////////////////////////////////////////////

    SchmittTrigger extClockTrigger;

    SchmittTrigger startTriggerA;
    SchmittTrigger startTriggerB;
    SchmittTrigger startTriggerC;
    SchmittTrigger startTriggerD;

    SchmittTrigger stopTriggerA;
    SchmittTrigger stopTriggerB;
    SchmittTrigger stopTriggerC;
    SchmittTrigger stopTriggerD;

    SchmittTrigger resetTriggerA;
    SchmittTrigger resetTriggerB;
    SchmittTrigger resetTriggerC;
    SchmittTrigger resetTriggerD;


    SchmittTrigger globalStartTriger;
    SchmittTrigger globalStopTriger;
    SchmittTrigger globalResetTriger;

    SchmittTrigger touchTriger [32];


///////////////////////////////////////////////////////////////////////////////
// Pulse Genearators
///////////////////////////////////////////////////////////////////////////////

    PulseGenerator gatePulseGeneratorA;
    PulseGenerator gatePulseGeneratorB;
    PulseGenerator gatePulseGeneratorC;
    PulseGenerator gatePulseGeneratorD;

    PulseGenerator pitchPulseGeneratorA;
    PulseGenerator pitchPulseGeneratorB;
    PulseGenerator pitchPulseGeneratorC;
    PulseGenerator pitchPulseGeneratorD;

    PulseGenerator lastPulseGeneratorA;
    PulseGenerator lastPulseGeneratorB;
    PulseGenerator lastPulseGeneratorC;
    PulseGenerator lastPulseGeneratorD;


	PulseGenerator clockPulseGenerator;
	PulseGenerator bpmPulseGenerator;

	PulseGenerator notePulseGeneratorA;
	PulseGenerator notePulseGeneratorB;
	PulseGenerator notePulseGeneratorC;
	PulseGenerator notePulseGeneratorD;


///////////////////////////////////////////////////////////////////////////////

    Scorovnik();
    //~Scorovnik();
    void step() override;
    //void onSampleRateChange() override;
    //void reset() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

///////////////////////////////////////////////////////////////////////////////


struct ScorovnikWidget : ModuleWidget {
    ScorovnikWidget(Scorovnik *module);
    void appendContextMenu(Menu *menu) override;
};

///////////////////////////////////////////////////////////////////////////////

#endif