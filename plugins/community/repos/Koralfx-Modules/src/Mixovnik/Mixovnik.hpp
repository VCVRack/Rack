/*
Mixovnik.hpp
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

#ifndef KORALFX_MIXOVNIK_HPP
#define KORALFX_MIXOVNIK_HPP

#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

#define PI_4 0.78539816339
#define SQRT2_2 0.70710678118

struct Mixovnik : Module {
	enum ParamIds {
		AUX1_VOLUME,
		AUX2_VOLUME,
		MIX_L_VOLUME,
		MIX_R_VOLUME,
		MIX_LINK,
		AUX1_MUTE,
		AUX2_MUTE,
		MIX_L_MUTE,
		MIX_R_MUTE,
		LINK_PARAM,
		PAN_PARAM  			= LINK_PARAM + 8,
		AUX1_PARAM 			= PAN_PARAM + 16,
		AUX2_PARAM 			= AUX1_PARAM + 16,
		VOLUME_PARAM 		= AUX2_PARAM + 16,
		MUTE_PARAM 			= VOLUME_PARAM + 16,
		SOLO_PARAM 			= MUTE_PARAM + 16,
		NUM_PARAMS 			= SOLO_PARAM + 16
	};
	enum InputIds {
		STEREO_INPUT_L,
		STEREO_INPUT_R,
		AUX1_INPUT_L,
		AUX1_INPUT_R,
		AUX2_INPUT_L,
		AUX2_INPUT_R,
		STRIPE_INPUT,
		STRIPE_CV_PAN_INPUT = STRIPE_INPUT + 16,
		STRIPE_CV_VOL_INPUT = STRIPE_CV_PAN_INPUT + 16,
		NUM_INPUTS 			= STRIPE_CV_VOL_INPUT + 16
	};
	enum OutputIds {
		STEREO_OUTPUT_L,
		STEREO_OUTPUT_R,
		AUX1_OUTPUT_L,
		AUX1_OUTPUT_R,
		AUX2_OUTPUT_L,
		AUX2_OUTPUT_R,
		NUM_OUTPUTS
	};
	enum LightIds {
		MIX_LIGHT_L,
		MIX_LIGHT_R,
		AUX1_LIGHT,
		AUX2_LIGHT,
		SIGNAL_LIGHT_NORMAL,
		SIGNAL_LIGHT_OVER,
		NUM_LIGHTS 			= SIGNAL_LIGHT_NORMAL + 32
	};

    int panelStyle 			= 0;
    float antiPop [16]		= {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float antiPopCurrentSpeed [16]		= {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float antiPopMixLeft	= 0.0f;
    float antiPopMixRight	= 0.0f;
    float antiPopAux1		= 0.0f;
    float antiPopAux2		= 0.0f;
    float antiPopSpeed		= 0.0005f;
///////////////////////////////////////////////////////////////////////////////

    Mixovnik();
    //~Mixovnik();
    void step() override;
    //void onSampleRateChange() override;
    //void reset() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

};

///////////////////////////////////////////////////////////////////////////////

struct MixovnikWidget : ModuleWidget {
    MixovnikWidget(Mixovnik *module);
    void appendContextMenu(Menu *menu) override;
};

///////////////////////////////////////////////////////////////////////////////

#endif