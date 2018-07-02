/*
Quantovnik.hpp
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

#ifndef KORALFX_QUANTOVNIK_HPP
#define KORALFX_QUANTOVNIK_HPP

#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

struct Quantovnik : Module {
	enum ParamIds {
		OCTAVE_PARAM,
		COARSE_PARAM,
		CV_IN_PARAM,
		CV_OUT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_PITCH_INPUT,
		CV_COARSE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NOTE_LIGHT,
		OCTAVE_LIGHT 	= NOTE_LIGHT + 12,
		NUM_LIGHTS 		= OCTAVE_LIGHT + 7
	};

///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    int panelStyle 		= 0;

///////////////////////////////////////////////////////////////////////////////

    Quantovnik();
    //~Quantovnik();
    void step() override;
    //void onSampleRateChange() override;
    //void reset() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

///////////////////////////////////////////////////////////////////////////////

struct QuantovnikWidget : ModuleWidget {
    QuantovnikWidget(Quantovnik *module);
    void appendContextMenu(Menu *menu) override;
};

///////////////////////////////////////////////////////////////////////////////

#endif