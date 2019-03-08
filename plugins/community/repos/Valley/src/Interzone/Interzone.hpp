//
// Interzone.hpp
// Author: Dale Johnson
// Contact: valley.audio.soft@gmail.com
// Date: 8/2/2019
//
// Copyright 2018 Dale Johnson. Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 1. Redistributions of
// source code must retain the above copyright notice, this list of conditions and the following
// disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or other materials
// provided with the distribution. 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this software without
// specific prior written permission.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef DSJ_INTERZONE_HPP
#define DSJ_INTERZONE_HPP

#include <pmmintrin.h>
#include <iostream>
#include "../Valley.hpp"
#include "../ValleyComponents.hpp"
#include "../Common/DSP/OTAFilter.hpp"
#include "../Common/DSP/OnePoleFilters.hpp"
#include "../Common/DSP/DOsc.hpp"
#include "../Common/DSP/DLFO.hpp"
#include "../Common/DSP/DADSR.hpp"
#include "../Common/DSP/Noise.hpp"

struct Interzone : Module {
    enum InputIds {
        VOCT_INPUT_1,
        VOCT_INPUT_2,
        PW_MOD_INPUT,
        GATE_INPUT,
        TRIG_INPUT,
        EXT_INPUT,

        FILTER_CUTOFF_INPUT_1,
        FILTER_CUTOFF_INPUT_2,
        FILTER_RES_INPUT,
        FILTER_INPUT,

        LFO_RATE_INPUT,
        LFO_TRIG_INPUT,
        LFO_SYNC_INPUT,

        VCA_LEVEL_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        SAW_OUTPUT,
        PULSE_OUTPUT,
        SUB_OUTPUT,
        MIX_OUTPUT,
        FILTER_OUTPUT,
        VCA_OUTPUT,

        LFO_SINE_OUTPUT,
        LFO_TRI_OUTPUT,
        LFO_SAW_UP_OUTPUT,
        LFO_SAW_DOWN_OUTPUT,
        LFO_PULSE_OUTPUT,
        LFO_SH_OUTPUT,
        LFO_NOISE_OUTPUT,

        ENV_POSITIVE_OUTPUT,
        ENV_NEGATIVE_OUTPUT,
        NUM_OUTPUTS
    };

    enum ParamIds {
        OCTAVE_PARAM,
        COARSE_PARAM,
        FINE_PARAM,
        PITCH_MOD_PARAM,
        PITCH_MOD_ENV_POL_PARAM,
        PITCH_MOD_SOURCE_PARAM,
        PW_PARAM,
        PW_MOD_PARAM,
        PW_MOD_SOURCE_PARAM,
        PW_MOD_ENV_POL_PARAM,
        COARSE_MODE_PARAM,
        GLIDE_PARAM,
        SUB_OCTAVE_PARAM,
        SUB_WAVE_PARAM,
        NOISE_TYPE_PARAM,

        SAW_LEVEL_PARAM,
        PULSE_LEVEL_PARAM,
        SUB_LEVEL_PARAM,
        NOISE_LEVEL_PARAM,
        EXT_LEVEL_PARAM,

        FILTER_CUTOFF_PARAM,
        FILTER_Q_PARAM,
        FILTER_HPF_PARAM,
        FILTER_POLES_PARAM,
        FILTER_MOD_PARAM,
        FILTER_VOCT_PARAM,
        FILTER_ENV_PARAM,
        FILTER_ENV_POL_PARAM,
        FILTER_CV_1_PARAM,
        FILTER_CV_2_PARAM,

        LFO_RATE_PARAM,
        LFO_FINE_PARAM,
        LFO_SLEW_PARAM,
        LFO_WAVE_PARAM,

        ENV_ATTACK_PARAM,
        ENV_DECAY_PARAM,
        ENV_SUSTAIN_PARAM,
        ENV_RELEASE_PARAM,
        ENV_LENGTH_PARAM,
        ENV_CYCLE_PARAM,
        ENV_MANUAL_PARAM,

        VCA_SOURCE_PARAM,
        VCA_LEVEL_CV_PARAM,
        NUM_PARAMS
    };

    enum LightIds {
        LFO_LIGHT,
        ENV_LIGHT,
        NUM_LIGHTS
    };

    Interzone();
    void step() override;
    void onSampleRateChange() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    float pitch;
    float pwm;
    float oscPitchMod;
    float filterCutoff;
    float lfoValue;
    float gateLevel;

    OnePoleLPFilter glide;
    DOsc osc;
    float noise;
    float subWave;
    float mix;
    OTAFilter filter;
    OnePoleHPFilter highpass;
    float outputLevel;
    float output;

    float sampleAndHold;
    DLFO lfo;
    PinkNoise pink;
    OnePoleLPFilter lfoSlew;
    OnePoleLPFilter gateSlew;
    DEnv env;

    int panelStyle = 0;
};

struct InterzonePanelStyleItem : MenuItem {
    Interzone* module;
    int panelStyle;
    void onAction(EventAction &e) override;
    void step() override;
};

struct InterzoneWidget : ModuleWidget {
    InterzoneWidget(Interzone *module);
    void appendContextMenu(Menu *menu);

    float octaveMinAngle = -0.222222f * M_PI;
    float octaveMaxAngle = 0.222222f * M_PI;
    float lfoWaveMinAngle = -0.333333f * M_PI;
    float lfoWaveMaxAngle = 0.333333f * M_PI;

    // Control Positions
    Vec VCOGlideSliderPos = Vec(13.15f, 43.6f);
    Vec VCOModSliderPos = Vec(34.f, 43.6f);
    Vec VCOOctavePos = Vec(123.212f, 64.674f);
    Vec VCOCoarsePos = Vec(114.584f, 113.266f);
    Vec VCOFinePos = Vec(144.584f, 113.266f);
    Vec VCOWidthSliderPos = Vec(171.15f, 43.6f);
    Vec VCOPWMSliderPos = Vec(191.95f, 43.6f);
    Vec VCOPWMSourcePos = Vec(222.742f, 89.345f);
    Vec VCOPWMEnvPolPos = Vec(222.888f, 51.172f);
    Vec VCOModEnvPolPos = Vec(64.498f, 51.172f);
    Vec VCOModSourcePos = Vec(64.498f, 99.173f);
    Vec VCOCoarseModePos = Vec(88.489f, 51.172f);
    Vec VCOSubOctPos = Vec(336.05f, 43.6f);

    Vec MixerSawLevelPos = Vec(270.15f, 43.6f);
    Vec MixerPulseLevelPos = Vec(291.15f, 43.6f);
    Vec MixerSubLevelPos = Vec(312.15f, 43.6f);
    Vec MixerSubWavePos = Vec(373.3f, 102.85f);
    Vec MixerNoiseTypePos = Vec(399.481f, 56.632f);
    Vec MixerNoiseLevelPos = Vec(426.15f, 43.6f);
    Vec MixerExtInLevelPos = Vec(447.15f, 43.6f);

    Vec FilterCutoffPos = Vec(13.f, 172.5f);
    Vec FilterResPos = Vec(34.f, 172.5f);
    Vec FilterHPFPos = Vec(55.f, 172.5f);
    Vec FilterPolesPos = Vec(84.443f, 240.504f);
    Vec FilterEnvPos = Vec(108.15f, 172.5f);
    Vec FilterLFOPos = Vec(129.25f, 172.5f);
    Vec FilterVOctPos = Vec(150.25f, 172.5f);
    Vec FilterEnvPolPos = Vec(84.443f, 184.085f);
    Vec FilterCV1Pos = Vec(182.554f, 284.4f);
    Vec FilterCV2Pos = Vec(182.554f, 320.567f);

    Vec LFORatePos = Vec(186.25f, 172.5f);
    Vec LFOFinePos = Vec(214.862f, 242.066f);
    Vec LFOSlewPos = Vec(240.584f, 242.066f);
    Vec LFOWavePos = Vec(234.612f, 194.074f);

    Vec EnvAttackPos = Vec(342.15f, 172.5f);
    Vec EnvDecayPos = Vec(363.15f, 172.5f);
    Vec EnvSustainPos = Vec(384.15f, 172.5f);
    Vec EnvReleasePos = Vec(405.15f, 172.5f);
    Vec EnvLengthPos = Vec(319.497f, 180.663f);
    Vec EnvCyclePos = Vec(319.497f, 240.665f);
    Vec EnvManualPos = Vec(292.844f, 245.275f);

    Vec VCASourcePos = Vec(447.483f, 181.663f);
    Vec VCALevelPos = Vec(250.414f, 283.9f);

    // IO Positions
    float jackRow1Y = 283.667f;
    float jackRow2Y = 319.667f;
    Vec VOctIn1Pos = Vec(22.154, jackRow1Y);
    Vec VOctIn2Pos = Vec(46.254, jackRow1Y);
    Vec PWMInPos = Vec(70.154, jackRow1Y);
    Vec SawOutPos = Vec(22.154, jackRow2Y);
    Vec PulseOutPos = Vec(46.254, jackRow2Y);
    Vec SubOutPos = Vec(70.154, jackRow2Y);

    Vec MixerExtInPos = Vec(115.154f, jackRow1Y);
    Vec MixerOutPos = Vec(115.154f, jackRow2Y);

    Vec FilterCutoffIn1Pos = Vec(158.154f, jackRow1Y);
    Vec FilterCutoffIn2Pos = Vec(158.154f, jackRow2Y);
    Vec FilterResInPos = Vec(206.154f, jackRow1Y);
    Vec FilterOutPos = Vec(206.154f, jackRow2Y);

    Vec LFORateInPos = Vec(291.154f, jackRow1Y);
    Vec LFOTrigInPos = Vec(315.154f, jackRow1Y);
    Vec LFOSHOutPos = Vec(339.154f, jackRow1Y);
    Vec LFONoiseOutPos = Vec(363.154f, jackRow1Y);
    Vec LFOPulseOutPos = Vec(387.154f, jackRow1Y);

    Vec LFOSyncInPos = Vec(291.154f, jackRow2Y);
    Vec LFOSineOutPos = Vec(315.154f, jackRow2Y);
    Vec LFOTriOutPos = Vec(339.154f, jackRow2Y);
    Vec LFOSawUpPos = Vec(363.154f, jackRow2Y);
    Vec LFOSawDownPos = Vec(387.154f, jackRow2Y);

    Vec EnvGateInPos = Vec(429.154f, jackRow1Y);
    Vec EnvTrigInPos = Vec(453.154f, jackRow1Y);
    Vec EnvPositiveOutPos = Vec(429.154f, jackRow2Y);
    Vec EnvNegativeOutPos = Vec(453.154f, jackRow2Y);

    Vec VCAOutPos = Vec(444.154f, 241.667f);
    Vec VCALevelCVPos = Vec(249.068, jackRow2Y);
};

#endif
