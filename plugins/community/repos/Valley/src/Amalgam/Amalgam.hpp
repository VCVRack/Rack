//
// Amalgam.hpp
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

#ifndef DSJ_AMALGAM_HPP
#define DSJ_AMALGAM_HPP

#include "../Valley.hpp"
#include "../ValleyComponents.hpp"
#include "DiodeRingMod.hpp"
#include "VecAmalgam.hpp"
#include "../Common/DSP/OnePoleFilters.hpp"
#include "../Common/DSP/NonLinear.hpp"
#include "../Common/SIMD/VecNonLinear.hpp"
#include <vector>
#include <cstdint>
#include <vector>
#include <cmath>

#define _1_INT32_MAX 1 / INT32_MAX
using namespace std;

struct Amalgam : Module {

    enum InputIds {
        X_LEFT_INPUT,
        X_RIGHT_INPUT,
        Y_LEFT_INPUT,
        Y_RIGHT_INPUT,

        X_GAIN_CV_INPUT,
        Y_GAIN_CV_INPUT,

        MODE_CV_INPUT,
        TYPE_CV1_INPUT,
        TYPE_CV2_INPUT,
        PARAM_A_CV1_INPUT,
        PARAM_A_CV2_INPUT,
        PARAM_B_CV1_INPUT,
        PARAM_B_CV2_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        X_Y_LEFT_AND_OUTPUT,
        X_Y_LEFT_XOR_OUTPUT,
        X_Y_RIGHT_AND_OUTPUT,
        X_Y_RIGHT_XOR_OUTPUT,
        Z_LEFT_OUTPUT,
        Z_RIGHT_OUTPUT,
        Z_AND_OUTPUT,
        Z_XOR_OUTPUT,
        Z_LEFT_PULSE_1_OUTPUT,
        Z_RIGHT_PULSE_1_OUTPUT,
        Z_LEFT_PULSE_2_OUTPUT,
        Z_RIGHT_PULSE_2_OUTPUT,
        NUM_OUTPUTS
    };

    enum ParamIds {
        X_GAIN,
        Y_GAIN,
        A_PARAM,
        B_PARAM,
        TYPE_PARAM,
        TYPE_CV1_PARAM,
        TYPE_CV2_PARAM,
        X_GAIN_CV_PARAM,
        Y_GAIN_CV_PARAM,
        PARAM_A_CV1_PARAM,
        PARAM_A_CV2_PARAM,
        PARAM_B_CV1_PARAM,
        PARAM_B_CV2_PARAM,
        DC_COUPLE_PARAM,
        NUM_PARAMS
    };

    enum LightIds {
        DC_COUPLE_LIGHT,
        NUM_LIGHTS
    };

    std::string modeNames[VecAmalgam::NUM_MODES] = {
        "RINGMOD1", "RINGMOD2", "RINGMOD3", "DIODE-RM", "MINIMAXI",
        "SIGN-1", "SIGN-2", "X-FADE", "FLIPFLOP", "ALPHAPWM", "BITAND",
        "BITXOR", "BITINTLV", "BITGLTCH", "FL32-AND", "FL32INTV", "FL32HACK"
    };

    std::string paramANames[VecAmalgam::NUM_MODES] = {
        "QUADRANT", "FOLD-X", "FOLDXY", "OFFVLT", "MIN<>MAX",
        "MIDPOINT", "MIDPOINT", "BIAS", "FAVOUR", "PULSES", "BITDEPTH", "BITDEPTH",
        "BITDEPTH", "JITTER", "JITTER", "BITDEPTH", "JITTER"
    };

    std::string paramBNames[VecAmalgam::NUM_MODES] = {
        "XPWM", "FOLD-Y", "XORBLEND", "ON-VLT", "DIFF*Z",
        "THRESH", "THRESH", "XMOD", "THRESH", "PWIDTH", "BITCRUSH", "BITCRUSH",
        "BITCRUSH", "BITCRUSH", "BITCRUSH", "BITCRUSH", "BITCRUSH"
    };

    __m128 __zeros, __fives, __halfs;
    __m128 __x, __y, __z;
    __m128 __xyAnd, __xyXor;
    __m128 __zPls1Mask, __zPls1, __zPls2;
    float xGain, yGain;
    float xIn[2];
    float yIn[2];
    float zOut[4];
    float zPls1[4];
    float zPls2[4];
    float andOut[4];
    float xorOut[4];
    float zAnd, zXor;

    float xx = 0.f;
    float driveOut = 0.f;

    VecAmalgam vecAmalgam;
    VecOnePoleHPFilter xyAndDCFilter, xyXorDCFilter;
    VecOnePoleHPFilter zOutDCFilter, zPls1DCFilter, zPls2DCFilter;
    OnePoleHPFilter zAndDCFilter, zXorDCFilter;

    float paramA = 0.f;
    float paramB = 0.f;

    float amalgamType;
    int iAmalgamType;
    int textColor = 2;

    int crossModMode = 0;
    bool dcCoupleButtonState = false;
    bool prevDcCoupleButtonState = false;
    bool dcCoupled = false;
    int panelStyle = 0;

    Amalgam();
    void step() override;
    void onSampleRateChange() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};

struct AmalgamPanelStyleItem : MenuItem {
    Amalgam* module;
    int panelStyle;
    void onAction(EventAction &e) override;
    void step() override;
};

struct AmalgamWidget : ModuleWidget {
    AmalgamWidget(Amalgam *module);
    void appendContextMenu(Menu *menu);

    // Control positions
    Vec xDriveKnobPos = Vec(50.1, 53.083);
    Vec yDriveKnobPos = Vec(80.164, 53.083);
    Vec modeKnobPos = Vec(63.095, 28.095);
    Vec typeKnobPos = Vec(59.112, 128.146);
    Vec paramAKnobPos = Vec(59.112, 197.074);
    Vec paramBKnobPos = Vec(59.112, 269.146);

    // Attenuveter positions
    Vec xDriveCVKnobPos = Vec(5.035, 73.033);
    Vec yDriveCVKnobPos = Vec(125.035, 73.133);
    Vec modeCV1KnobPos = Vec(35.135, 152.133);
    Vec modeCV2KnobPos = Vec(95.135, 152.133);

    Vec paramACV1KnobPos = Vec(29.135, 197.133);
    Vec paramACV2KnobPos = Vec(101.135, 197.133);
    Vec paramBCV1KnobPos = Vec(8.135, 260.033);
    Vec paramBCV2KnobPos = Vec(122.135, 260.033);

    // Input jack positions
    Vec xLeftInputPos = Vec(4.654, 28.367);
    Vec xRightInputPos = Vec(31.654, 28.367);
    Vec yLeftInputPos = Vec(97.654, 28.367);
    Vec yRightInputPos = Vec(124.654, 28.367);

    Vec xDriveCVInputPos = Vec(4.654, 99.7);
    Vec yDriveCVInputPos = Vec(124.654, 99.7);
    Vec modeCVInputPos = Vec(64.654, 70.7);
    Vec typeCV1InputPos = Vec(4.654, 162.667);
    Vec typeCV2InputPos = Vec(124.654, 162.667);
    Vec paramACV1InputPos = Vec(4.654, 205.667);
    Vec paramACV2InputPos = Vec(124.654, 205.667);
    Vec paramBCV1InputPos = Vec(4.654, 232.7);
    Vec paramBCV2InputPos = Vec(124.654, 232.7);

    // Output jack positions
    Vec leftANDOutputPos = Vec(4.654, 125.667);
    Vec leftXOROutputPos = Vec(34.654, 115.667);
    Vec rightANDOutputPos = Vec(124.654, 125.667);
    Vec rightXOROutputPos = Vec(94.654, 115.667);

    Vec zLeftOutputPos = Vec(52.654, 319.2);
    Vec zRightOutputPos = Vec(76.654, 319.2);
    Vec zANDOutputPos = Vec(16.654, 284.716);
    Vec zXOROutputPos = Vec(112.718, 284.716);
    Vec zLeftPulseOutputPos = Vec(28.645, 319.2);
    Vec zLeftPulse2OutputPos = Vec(4.645, 319.2);
    Vec zRightPulseOutputPos = Vec(100.654, 319.2);
    Vec zRightPulse2OutputPos = Vec(124.654, 319.2);

    Vec DCCoupleLightPos = Vec(59.044, 31.775);
};

#endif
