//
// Osc4Core_SIMD.hpp
// Author: Dale Johnson
// Contact: valley.audio.soft@gmail.com
// Date: 18/3/2018
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

#ifndef CLASS_DSJ_FM_CORE_HPP
#define CLASS_DSJ_FM_CORE_HPP

#include <cmath>
#include <string>
#include "QuadOsc.hpp"
#include "opal_wavetable/opal_wavetable.h"
#include "TeeEks_wavetable/TeeEks_wavetable.h"
#include "basic_wavetable/basic_wavetable.h"
#include "additiveBank_wavetable/additiveBank_wavetable.h"
#include "additiveSine_wavetable/additiveSine_wavetable.h"
#include "additiveSaw_wavetable/additiveSaw_wavetable.h"
#include "additiveSquare_wavetable/additiveSquare_wavetable.h"
#include "sweepHarmonic_wavetable/sweepHarmonic_wavetable.h"
#include "sineHarmonics_wavetable/sineHarmonics_wavetable.h"
#include "amHarmonic_wavetable/amHarmonic_wavetable.h"
#include "altosax_wavetable/altosax_waveTable.h"
#include "cello1_wavetable/cello1_waveTable.h"
#include "cello2_wavetable/cello2_waveTable.h"
#include "violin_wavetable/violin_waveTable.h"
#include "oboe_wavetable/oboe_waveTable.h"
#include "piano_wavetable/piano_waveTable.h"
#include "pluck_wavetable/pluck_waveTable.h"
#include "theremin_wavetable/theremin_waveTable.h"
#include "overtone1_wavetable/overtone1_waveTable.h"
#include "overtone2_wavetable/overtone2_waveTable.h"
#include "symetric_wavetable/symetric_waveTable.h"
#include "voice1_wavetable/voice1_waveTable.h"
#include "voice2_wavetable/voice2_waveTable.h"
#include "voice3_wavetable/voice3_waveTable.h"
#include "voice4_wavetable/voice4_waveTable.h"
#include "voice5_wavetable/voice5_waveTable.h"
#include "chip1_wavetable/chip1_waveTable.h"
#include "chip2_wavetable/chip2_waveTable.h"
#include "bitcrush1_wavetable/bitcrush1_waveTable.h"
#include "bitcrush2_wavetable/bitcrush2_waveTable.h"
#include "pwm_wavetable/pwm_wavetable.h"
#include "biPulse_wavetable/biPulse_wavetable.h"
#include "sawGap_wavetable/sawGap_wavetable.h"
#include "sawGap2_wavetable/sawGap2_wavetable.h"
#include "vgame_wavetable/vgame_wavetable.h"

#define NUM_WAVEBANKS 35

const int kNumOperators = 4;
const int kNumAlgorithms = 23;

static float** wavetables[NUM_WAVEBANKS] = {
    wavetable_opal, wavetable_basic, wavetable_TeeEks, wavetable_sineHarmonics, wavetable_additiveSine, wavetable_amHarmonic,
    wavetable_sweepHarmonic, wavetable_additiveSaw, wavetable_additiveSquare, wavetable_additiveBank,
    wavetable_oboe, wavetable_altosax, wavetable_cello1, wavetable_cello2, wavetable_violin,
    wavetable_piano, wavetable_theremin, wavetable_pluck, wavetable_overtone1,
    wavetable_overtone2, wavetable_symetric, wavetable_chip1, wavetable_chip2,
    wavetable_bitcrush1, wavetable_bitcrush2, wavetable_voice1, wavetable_voice2,
    wavetable_voice3, wavetable_voice4, wavetable_voice5, wavetable_pwm,
    wavetable_biPulse, wavetable_sawGap, wavetable_sawGap2, wavetable_vgame
};

static long* wavetable_lengths[NUM_WAVEBANKS] = {
    wavetable_opal_lengths, wavetable_basic_lengths, wavetable_TeeEks_lengths, wavetable_sineHarmonics_lengths, wavetable_additiveSine_lengths, wavetable_amHarmonic_lengths,
    wavetable_sweepHarmonic_lengths, wavetable_additiveSaw_lengths, wavetable_additiveSquare_lengths, wavetable_additiveBank_lengths,
    wavetable_oboe_lengths, wavetable_altosax_lengths, wavetable_cello1_lengths, wavetable_cello2_lengths, wavetable_violin_lengths,
    wavetable_piano_lengths, wavetable_theremin_lengths, wavetable_pluck_lengths, wavetable_overtone1_lengths,
    wavetable_overtone2_lengths, wavetable_symetric_lengths, wavetable_chip1_lengths, wavetable_chip2_lengths,
    wavetable_bitcrush1_lengths, wavetable_bitcrush2_lengths, wavetable_voice1_lengths, wavetable_voice2_lengths,
    wavetable_voice3_lengths, wavetable_voice4_lengths, wavetable_voice5_lengths, wavetable_pwm_lengths,
    wavetable_biPulse_lengths, wavetable_sawGap_lengths, wavetable_sawGap2_lengths, wavetable_vgame_lengths
};

static int32_t wavetable_sizes[NUM_WAVEBANKS] = {
    WAVETABLE_OPAL_NUM, WAVETABLE_BASIC_NUM, WAVETABLE_TEEEKS_NUM, WAVETABLE_SINEHARMONICS_NUM, WAVETABLE_ADDITIVESINE_NUM, WAVETABLE_AMHARMONIC_NUM,
    WAVETABLE_SWEEPHARMONIC_NUM, WAVETABLE_ADDITIVESAW_NUM, WAVETABLE_ADDITIVESQUARE_NUM, WAVETABLE_ADDITIVEBANK_NUM,
    WAVETABLE_OBOE_NUM, WAVETABLE_ALTOSAX_NUM, WAVETABLE_CELLO1_NUM, WAVETABLE_CELLO2_NUM, WAVETABLE_VIOLIN_NUM,
    WAVETABLE_PIANO_NUM, WAVETABLE_THEREMIN_NUM, WAVETABLE_PLUCK_NUM, WAVETABLE_OVERTONE1_NUM,
    WAVETABLE_OVERTONE2_NUM, WAVETABLE_SYMETRIC_NUM, WAVETABLE_CHIP1_NUM, WAVETABLE_CHIP2_NUM,
    WAVETABLE_BITCRUSH1_NUM, WAVETABLE_BITCRUSH2_NUM, WAVETABLE_VOICE1_NUM, WAVETABLE_VOICE2_NUM,
    WAVETABLE_VOICE3_NUM, WAVETABLE_VOICE4_NUM, WAVETABLE_VOICE5_NUM, WAVETABLE_PWM_NUM,
    WAVETABLE_BIPULSE_NUM, WAVETABLE_SAWGAP_NUM, WAVETABLE_SAWGAP2_NUM, WAVETABLE_VGAME_NUM
};

static std::string waveTableNames[NUM_WAVEBANKS] = {
    "Opal", "Basic", "TeeEks", "SinHarm", "AddSin", "AMHarm", "SwpHarm", "AddSaw", "AddSqr", "AddBank", "Oboe", "Sax", "Cello1",
    "Cello2", "Violin", "Piano", "Thrmin", "Pluck", "OvrTne1", "OvrTne2", "Sym", "Chip1",
    "Chip2", "BitCrsh1", "BitCrsh2", "Voice1", "Voice2", "Voice3", "Voice4", "Voice5", "PWM",
    "BiPls", "SawGap1", "SawGap2", "VGame"
};

class FourVoiceOPCore {
public:
    enum OpSyncSource {
        PARENT_SYNC_SOURCE,
        NEIGHBOUR_SYNC_SOURCE
    };

    FourVoiceOPCore();
    FourVoiceOPCore(const FourVoiceOPCore& copy) = delete;
    void process();
    void resetPhase();
    void externalFM(int opNum, float extFM);
    void externalSync(int opNum, float extSync);

    __m128 getMainOutput() const;
    __m128 getBOutput() const;
    __m128 getOpOutput(int opNum) const;
    void setAlgorithm(int newAlgorithm);
    void setFeedback(float feedback);
    void setFrequency(int opNum, float frequency);
    void _mm_setFrequency(int opNum, const __m128& frequency);
    void setWavebank(int opNum, int bankNum);
    void setWavePosition(int opNum, float position);
    void setShape(int opNum, float shape);
    void setLevel(int opNum, float level);
    void setOpPreFade(int opNum, bool opPreFade);
    void setBrightness(float brightness);
    void setSyncMode(int opNum, int syncMode);
    void setSyncSource(OpSyncSource opSyncSource);
    void enableSync(int opNum, bool enableSync);
    void enableIntSync(int opNum, bool enableIntSync);
    void enableWeakSync(int opNum, bool weakEnable);
    void setShapeMode(int opNum, int shapeMode);
    void setPMPostShape(int opNum, bool PMPostShape);
    void setSampleRate(float sampleRate);

private:
    ScanningQuadOsc _op[4];

    enum MatrixRows{
         OP_0_ROW,
         OP_1_ROW,
         OP_2_ROW,
         OP_3_ROW,
         NUM_ROWS
     };

    enum MatrixColumns{
         OP_0_COL,
         OP_1_COL,
         OP_2_COL,
         OP_3_COL,
         MAIN_OUT_COL,
         B_OUT_COL,
         NUM_COLS
    };

    float _inLevels[4];
    float _opLevels[4];
    bool _opPreFade[4];
    __m128 __opLevel[4];
    __m128 __opOut[4];
    __m128 __opAuxOut[4];
    __m128 __op1Eoc, __op2Eoc, __op3Eoc, __op4Eoc;
    __m128 __opExtFM[4];
    __m128 __opExtSync[4];
    __m128 __opSyncEnable[4];

    __m128 __matrix[NUM_ROWS][NUM_COLS]; // Row = Source, Col = Dest
    __m128 __op1Col, __op2Col, __op3Col, __op4Col, __mainCol, __bCol;
    OpSyncSource _opSyncSource;
    __m128 __opSyncSignal[4];
    __m128 __opSyncIn[4];
    __m128 __ones, __zeros, __five;
    __m128 __outputLevels[4];
    __m128 __aOutLevel, __bOutLevel;
    bool extSyncing;

    float _brightness;
    int _brightnessMask;
    int _algorithm;

    void calcOpLevels();
    void mix();
    void clearMatrix();

    // Combined output algorithms
    void setMatrixAlgo0();
    void setMatrixAlgo1();
    void setMatrixAlgo2();
    void setMatrixAlgo3();
    void setMatrixAlgo4();
    void setMatrixAlgo5();
    void setMatrixAlgo6();
    void setMatrixAlgo7();
    void setMatrixAlgo8();
    void setMatrixAlgo9();
    void setMatrixAlgo10();
    void setMatrixAlgo11();

    // Seperate output algorithms
    void setMatrixAlgo12();
    void setMatrixAlgo13();
    void setMatrixAlgo14();
    void setMatrixAlgo15();
    void setMatrixAlgo16();
    void setMatrixAlgo17();
    void setMatrixAlgo18();
    void setMatrixAlgo19();
    void setMatrixAlgo20();
    void setMatrixAlgo21();
    void setMatrixAlgo22();
};

#endif // CLASS_DSJ_FM_CORE_HPP
