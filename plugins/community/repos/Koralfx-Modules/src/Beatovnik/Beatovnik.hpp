/*
Beatovnik.hpp
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
*/

#ifndef KORALFX_BEATOVNIK_HPP
#define KORALFX_BEATOVNIK_HPP

#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

#include "dsp/digital.hpp"
#include <string>


struct Beatovnik : Module {
	enum ParamIds {
		WIDTH_PARAM,
		TIME_PARAM,
		TIME_T_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		BEAT2X_MUL_OUTPUT,
		BEAT4X_MUL_OUTPUT,
		BEAT2X_DIV_OUTPUT,
		BEAT4X_DIV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_LIGHT,
		CLOCK_LOCK_LIGHT,
		BEAT2X_MUL_LIGHT,
		BEAT4X_MUL_LIGHT,
		BEAT2X_DIV_LIGHT,
		BEAT4X_DIV_LIGHT,
		NOTE_LIGHT,
		NUM_LIGHTS 			= NOTE_LIGHT + 9
	};


///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    int 	panelStyle 		= 0;
    NVGcolor colorDisplay 	= nvgRGB(0xff, 0xcc, 0x00);

    bool 	inMemory 		= false;
    
    bool 	beatLock 		= false;
    float 	beatTime 		= 0;
    int 	beatCount 		= 0;
    int 	beatCountMemory = 0;
    float 	beatOld 		= 0;

    int 	stepper 		= 0;
    bool 	stepperInc 		= false;

    float 	gateWidth2xMul	= 0;
    float 	gateWidth4xMul	= 0;
    float 	gateWidth2xDiv	= 0;
    float 	gateWidth4xDiv	= 0;

    bool 	gateDec2xMul 	= false;
    bool 	gateDec4xMul 	= false;
    bool 	gateDec2xDiv 	= false;
    bool 	gateDec4xDiv 	= false;

	std::string tempo;
	SchmittTrigger clockTrigger;
	PulseGenerator LightPulseGenerator;

///////////////////////////////////////////////////////////////////////////////

    Beatovnik();
    //~Beatovnik();
    void step() override;
    //void onSampleRateChange() override;
    //void reset() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

///////////////////////////////////////////////////////////////////////////////


struct BeatovnikWidget : ModuleWidget {
    BeatovnikWidget(Beatovnik *module);
    void appendContextMenu(Menu *menu) override;
};

///////////////////////////////////////////////////////////////////////////////

#endif