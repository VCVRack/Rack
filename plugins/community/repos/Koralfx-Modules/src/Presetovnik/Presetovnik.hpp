/*
Presetovnik.hpp
Author: Tomek Sosnowski
Contact: koralfx@gmail.com
Date: 5/4/2018

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

#ifndef KORALFXVCV_PRESETOVNIK_HPP
#define KORALFXVCV_PRESETOVNIK_HPP

#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"
#include "dsp/digital.hpp"

struct Presetovnik : Module {
	enum ParamIds {
		KNOB_PARAM,
		LED_BUTTON_PRESET_PARAM = KNOB_PARAM + 8,
		LED_UNI_PARAM  = LED_BUTTON_PRESET_PARAM + 10,
		NUM_PARAMS = LED_UNI_PARAM + 8
	};
	enum InputIds {
		CV_PRESET_INPUT,
		CV_PARAM_INPUT,
		NUM_INPUTS = CV_PARAM_INPUT + 8
	};
	enum OutputIds {
		CV_PRESET_OUTPUT,
		CV_PARAM_OUTPUT,
		NUM_OUTPUTS = CV_PARAM_OUTPUT + 8
	};
	enum LightIds {
		PRESET_LIGHT,
		UNI_LIGHT = PRESET_LIGHT + 10*3,
		NUM_LIGHTS = UNI_LIGHT + 8
	};

///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    int panelStyle 		= 0;

    float pointerKnob [8] = {10.0, 10.0,10.0,10.0,10.0,10.0,10.0,10.0};
    NVGcolor colorPointer [8] ={nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff)};
    float presetKnobMemory [10][8];
    bool presetUniMemory [10][8];

    int preset = 0;
    int presetOld = 0;
    float cvPresetInputOld =0;
    bool presetChange = true;
    bool cvParamsInputDisconnect [8];
    bool unipolarChange = false;
    bool cvPresetInputActiveOld= false;
    int sourcePreset = 2; //2=internal Preset, 1= CV Preset


	SchmittTrigger presetTrigger [10];
	SchmittTrigger unipolarTrigger [8];




///////////////////////////////////////////////////////////////////////////////

    Presetovnik();
    //~Presetovnik();
    void step() override;
    void onReset() override;
    //void onSampleRateChange() override;
    //void reset() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

///////////////////////////////////////////////////////////////////////////////

struct PresetovnikWidget : ModuleWidget {
    PresetovnikWidget(Presetovnik *module);
    void appendContextMenu(Menu *menu) override;
};

///////////////////////////////////////////////////////////////////////////////

#endif