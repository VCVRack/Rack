#include "Dexter.hpp"

Dexter::Dexter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    resetPhaseState = false;
    octave = 0;
    aPitch = 0.f;
    for(auto i = 0; i < kNumOperators; ++i) {
        opPitch[i] = 0.f;
        opLowFreq[i][0] = 440.f;
        opLowFreq[i][1] = 440.f;
        opLowFreq[i][2] = 440.f;
        opLowFreq[i][3] = 440.f;
        opHighFreq[i][0] = 440.f;
        opHighFreq[i][1] = 440.f;
        opHighFreq[i][2] = 440.f;
        opHighFreq[i][3] = 440.f;
        opLevel[i] = 0.f;
        opWave[i] = 0.f;
        opWaveBank[i] = 0.f;
        opButtonSync[i] = 0.f;
        opButtonWeakSync[i] = 0.f;
        opButtonPostShape[i] = 0.f;
        opButtonLFO[i] = 0.f;
        opButtonPreFade[i] = 0.f;
        opSync[i] = false;
        opWeakSync[i] = false;
        opPostShape[i] = false;
        opPreFade[i] = false;

        opMultipleKnob[i] = 0;
        opSettingsMenu[i] = 0;
        opWaveMenuVis[i] = 0;
        opModAMenuVis[i] = 0;
        opModBMenuVis[i] = 0;
        opMiscMenuVis[i] = 0;
    }
    brightness = 0.f;
    feedback = 0.f;
    allShape = 0.f;
    int NaN = 0xFFFFFFFF;
    __0000 = _mm_castsi128_ps(_mm_set1_epi32(0));
    __0001 = _mm_castsi128_ps(_mm_set_epi32(0,   0,   0,   NaN));
    __0011 = _mm_castsi128_ps(_mm_set_epi32(0,   0,   NaN, NaN));
    __0111 = _mm_castsi128_ps(_mm_set_epi32(0,   NaN, NaN, NaN));
    __1111 = _mm_castsi128_ps(_mm_set_epi32(NaN, NaN, NaN, NaN));

    __0010 = _mm_castsi128_ps(_mm_set_epi32(0,   0,   NaN, 0));
    __0110 = _mm_castsi128_ps(_mm_set_epi32(0,   NaN, NaN, 0));
    __1110 = _mm_castsi128_ps(_mm_set_epi32(NaN, NaN, NaN, 0));

    __bOutMask = __0001;
    __lowChordMask = __0010;
    __highChordMask = __0000;

    __lowChordMaskTable[0] = __0010;
    __lowChordMaskTable[1] = __0110;
    __lowChordMaskTable[2] = __1110;
    __lowChordMaskTable[3] = __1110;
    __lowChordMaskTable[4] = __1110;
    __lowChordMaskTable[5] = __1110;
    __lowChordMaskTable[6] = __1110;

    __highChordMaskTable[0] = __0000;
    __highChordMaskTable[1] = __0000;
    __highChordMaskTable[2] = __0000;
    __highChordMaskTable[3] = __0001;
    __highChordMaskTable[4] = __0011;
    __highChordMaskTable[5] = __0111;
    __highChordMaskTable[6] = __1111;

    __lowLeftGainTable[0] = _mm_set_ps(0.f,    0.f,    1.f, 0.f);
    __lowLeftGainTable[1] = _mm_set_ps(0.f,    0.f,    1.f, 0.f);
    __lowLeftGainTable[2] = _mm_set_ps(0.f,    0.5f,   1.f, 0.f);
    __lowLeftGainTable[3] = _mm_set_ps(0.333f, 0.666f, 1.f, 0.f);
    __lowLeftGainTable[4] = _mm_set_ps(0.5f,   0.75f,  1.f, 0.f);
    __lowLeftGainTable[5] = _mm_set_ps(0.6f,   0.8f,   1.f, 0.f);
    __lowLeftGainTable[6] = _mm_set_ps(0.666f, 0.833f, 1.f, 0.f);

    __lowRightGainTable[0] = _mm_set_ps(0.f,    0.f,    1.f,    0.f);
    __lowRightGainTable[1] = _mm_set_ps(0.f,    1.f,    0.f,    0.f);
    __lowRightGainTable[2] = _mm_set_ps(1.f,    0.5f,   0.f,    0.f);
    __lowRightGainTable[3] = _mm_set_ps(0.666f, 0.333f, 0.f,    0.f);
    __lowRightGainTable[4] = _mm_set_ps(0.5f,   0.25f,  0.f,    0.f);
    __lowRightGainTable[5] = _mm_set_ps(0.4f,   0.2f,   0.f,    0.f);
    __lowRightGainTable[6] = _mm_set_ps(0.333f, 0.166f, 0.f,    0.f);

    __highLeftGainTable[0] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highLeftGainTable[1] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highLeftGainTable[2] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highLeftGainTable[3] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highLeftGainTable[4] = _mm_set_ps(0.f,    0.f,    0.f,    0.25f);
    __highLeftGainTable[5] = _mm_set_ps(0.f,    0.f,    0.2f,   0.4f);
    __highLeftGainTable[6] = _mm_set_ps(0.f,    0.166f, 0.333f, 0.5f);

    __highRightGainTable[0] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highRightGainTable[1] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highRightGainTable[2] = _mm_set_ps(0.f,    0.f,    0.f,    0.f);
    __highRightGainTable[3] = _mm_set_ps(0.f,    0.f,    0.f,    1.f);
    __highRightGainTable[4] = _mm_set_ps(0.f,    0.f,    1.f,    0.75f);
    __highRightGainTable[5] = _mm_set_ps(0.f,    1.f,    0.8f,   0.6f);
    __highRightGainTable[6] = _mm_set_ps(1.f,    0.833f, 0.666f, 0.5f);

    chordKnob = 0;
    currentChord = 0;
    syncMode = 0;
    algo = 0;
    chordNotes = getChord(0, 0, 0, 0);
    pOpFreqs = (float*)aligned_alloc_16(sizeof(float) * 8);

    for(auto i = 0; i < 8; ++i) {
        pOpFreqs[i] = 0.f;
    }

    pLeftOut = (float*)aligned_alloc_16(sizeof(float) * 4);
    pRightOut = (float*)aligned_alloc_16(sizeof(float) * 4);
    pOP1 = (float*)aligned_alloc_16(sizeof(float) * 4);
    pOP2 = (float*)aligned_alloc_16(sizeof(float) * 4);
    pOP3 = (float*)aligned_alloc_16(sizeof(float) * 4);
    pOP4 = (float*)aligned_alloc_16(sizeof(float) * 4);
    pBOut = (float*)aligned_alloc_16(sizeof(float) * 4);
    for(auto i = 0; i < 4; ++i) {
        pLeftOut[i] = 0.f;
        pRightOut[i] = 0.f;
        pOP1[i] = 0.f;
        pOP2[i] = 0.f;
        pOP3[i] = 0.f;
        pOP4[i] = 0.f;
        pBOut[i] = 0.f;
    }
    __zeros = _mm_set1_ps(0.f);
    __leftOut = __zeros;
    __rightOut = __zeros;

    coreA.setSampleRate(engineGetSampleRate());
    coreB.setSampleRate(engineGetSampleRate());
}

Dexter::~Dexter() {
    aligned_free_16(pLeftOut);
    aligned_free_16(pRightOut);
    aligned_free_16(pOP1);
    aligned_free_16(pOP2);
    aligned_free_16(pOP3);
    aligned_free_16(pOP4);
    aligned_free_16(pBOut);
    aligned_free_16(pOpFreqs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// DSP ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void Dexter::step() {
    if(params[RESET_PHASE_BUTTON].value) {
        if(!resetPhaseState) {
            resetPhaseState = true;
            coreA.resetPhase();
            coreB.resetPhase();
        }
        lights[RESET_PHASE_LIGHT].value = 1.f;
    } else {
        resetPhaseState = false;
        lights[RESET_PHASE_LIGHT].value = 0.f;
    }

    if(masterLFOButtonTrig.process(params[MASTER_LFO_BUTTON].value)) {
        masterLFO = 1.f - masterLFO;
    }
    lights[MASTER_LFO_LIGHT].value = masterLFO ? 1.f : 0.f;

    if(fullInversionButtonTrig.process(params[FULL_INVERSION_BUTTON].value)) {
        buttonFullInversion = buttonFullInversion ? 0 : 1;
    }
    lights[FULL_INVERSION_LIGHT].value = buttonFullInversion ? 1.f : 0.f;

    // Menu system
    for(auto op = 0; op < kNumOperators; ++op) {
        if(opSettBtnTrig[op].process(params[opParams[op][OP_SETTINGS_PARAM]].value)) {
            opSettingsMenu[op] = !opSettingsMenu[op];
        }
        lights[opLights[op][OP_SETTINGS]].value = opSettingsMenu[op] ? 1.f : 0.f;

        if(opSyncBtnTrig[op].process(params[opParams[op][OP_SYNC_PARAM]].value)) {
            opButtonSync[op] = 1.f - opButtonSync[op];
        }
        lights[opLights[op][OP_SYNC]].value = opButtonSync[op] ? 1.f : 0.f;

        if(opPostShapeBtnTrig[op].process(params[opParams[op][OP_POST_SHAPE_PARAM]].value)) {
            opButtonPostShape[op] = 1.f - opButtonPostShape[op];
        }
        lights[opLights[op][OP_POST_SHAPE]].value = opButtonPostShape[op] ? 1.f : 0.f;

        if(opWeakBtnTrig[op].process(params[opParams[op][OP_WEAK_PARAM]].value)) {
            opButtonWeakSync[op] = 1.f - opButtonWeakSync[op];
        }
        lights[opLights[op][OP_WEAK]].value = opButtonWeakSync[op] ? 1.f : 0.f;

        if(opLFOBtnTrig[op].process(params[opParams[op][OP_LFO_PARAM]].value)) {
            opButtonLFO[op] = 1.f - opButtonLFO[op];
        }
        lights[opLights[op][OP_LFO]].value = (opButtonLFO[op] + masterLFO) ? 1.f : 0.f;

        if(opPreFadeBtnTrig[op].process(params[opParams[op][OP_PRE_PARAM]].value)) {
            opButtonPreFade[op] = 1.f - opButtonPreFade[op];
        }
        lights[opLights[op][OP_PRE_LIGHT]].value = opButtonPreFade[op] ? 1.f : 0.f;
    }

    for(auto op = 0; op < kNumOperators; ++op) {
        if(opWaveMenuBtnTrig[op].process(params[opParams[op][OP_WAVE_MENU_PARAM]].value)) {
            opMenuPage[op] = 0;
        }
        else if (opModAMenuBtnTrig[op].process(params[opParams[op][OP_MODA_MENU_PARAM]].value)) {
            opMenuPage[op] = 1;
        }
        else if (opModBMenuBtnTrig[op].process(params[opParams[op][OP_MODB_MENU_PARAM]].value)) {
            opMenuPage[op] = 2;
        }

        opWaveMenuVis[op] = 0;
        opModAMenuVis[op] = 0;
        opModBMenuVis[op] = 0;
        opMiscMenuVis[op] = 0;
        if(opSettingsMenu[op]) {
            switch(opMenuPage[op]) {
                case 0:
                    opWaveMenuVis[op] = 1;
                    break;
                case 1:
                    opModAMenuVis[op] = 1;
                    break;
                case 2:
                    opModBMenuVis[op] = 1;
            }
        }
        lights[opLights[op][OP_WAVE_LIGHT]].value = opWaveMenuVis[op] ? 1.f : 0.f;
        lights[opLights[op][OP_MODA_LIGHT]].value = opModAMenuVis[op] ? 1.f : 0.f;
        lights[opLights[op][OP_MODB_LIGHT]].value = opModBMenuVis[op] ? 1.f : 0.f;
    }

    brightness = params[BRIGHTNESS_PARAM].value;
    brightness += inputs[BRIGHTNESS_INPUT].value * 0.1f * params[BRIGHT_DEPTH_PARAM].value;
    brightness = clamp(brightness, -1.f, 1.f);
    allShape = params[SHAPE_PARAM].value + inputs[SHAPE_INPUT].value * 0.1f * params[SHAPE_DEPTH_PARAM].value;

    octaveAKnob = (int)params[OCTAVE_PARAM].value;
    aPitch = (float)(octaveAKnob - 3) + params[COARSE_PARAM].value + params[FINE_PARAM].value;
    aPitch += inputs[A_VOCT_INPUT].value;
    chordKnob = (int)params[CHORD_PARAM].value;
    chordParam = (float)chordKnob + (inputs[CHORD_INPUT].value * 0.1f * params[CHORD_DEPTH_PARAM].value * (float)(NUM_CHORDS - 1));
    chordParam = clamp(chordParam, 0.f, (float)NUM_CHORDS - 1.f);
    chordDetuneParam = params[DETUNE_PARAM].value + inputs[DETUNE_INPUT].value * 0.1f * params[DETUNE_DEPTH_PARAM].value;
    chordDetuneParam = clamp(chordDetuneParam, 0.f, 1.f) * -0.1f;
    invDepthKnob = (int)params[INVERT_PARAM].value;
    invDepthParam = invDepthKnob + (int)(inputs[INVERT_INPUT].value * params[INVERT_DEPTH_PARAM].value) - 10;
    invDepthParam = clamp(invDepthParam, -10, 10);
    if((Chords)((int)chordParam) != currentChord || invDepthParam != invDepth || chordDetuneParam != chordDetune
       || buttonFullInversion != fullInversion) {
        fullInversion = buttonFullInversion;
        chordDetune = chordDetuneParam;
        makeChord(chordParam, invDepthParam);
    }

    octaveBKnob = (int)params[B_OCTAVE_PARAM].value;
    bPitch = (float)(octaveBKnob - 3) + params[B_COARSE_PARAM].value + params[B_FINE_PARAM].value;
    bPitch += inputs[B_VOCT_INPUT].value;

    coreA.setSyncSource(opSyncSource);
    coreB.setSyncSource(opSyncSource);

    for(auto op = 0; op < kNumOperators; ++op) {
        modMatrix[op].setRowDestination(0, (RoutingMatrixDestination) opMod1Assign[op]);
        modMatrix[op].setRowDestination(1, (RoutingMatrixDestination) opMod2Assign[op]);
        modMatrix[op].setRowDestination(2, (RoutingMatrixDestination) opMod3Assign[op]);
        modMatrix[op].setRowDestination(3, (RoutingMatrixDestination) opMod4Assign[op]);

        modMatrix[op].setRowSourceValue(0, inputs[opCVInputs[op][OP_MOD_CV_1]].value * 0.1f);
        modMatrix[op].setRowSourceValue(1, inputs[opCVInputs[op][OP_MOD_CV_2]].value * 0.1f);
        modMatrix[op].setRowSourceValue(2, inputs[opCVInputs[op][OP_MOD_CV_3]].value * 0.1f);
        modMatrix[op].setRowSourceValue(3, inputs[opCVInputs[op][OP_MOD_CV_4]].value * 0.1f);

        modMatrix[op].setRowDepth(0, params[opCVAtten[op][OP_MOD_CV_1]].value);
        modMatrix[op].setRowDepth(1, params[opCVAtten[op][OP_MOD_CV_2]].value);
        modMatrix[op].setRowDepth(2, params[opCVAtten[op][OP_MOD_CV_3]].value);
        modMatrix[op].setRowDepth(3, params[opCVAtten[op][OP_MOD_CV_4]].value);

        modMatrix[op].process();

        opPitch[op] = params[opParams[op][OP_COARSE_PARAM]].value;
        opPitch[op] += params[opParams[op][OP_FINE_PARAM]].value;
        opPitch[op] += inputs[opCVInputs[op][OP_PITCH_CV_1]].value * params[opCVAtten[op][OP_PITCH_CV_1]].value;
        opPitch[op] += inputs[opCVInputs[op][OP_PITCH_CV_2]].value * params[opCVAtten[op][OP_PITCH_CV_2]].value;
        opPitch[op] += modMatrix[op].getDestinationValue(PITCH_DEST);
        opMultipleKnob[op] = (int)params[opParams[op][OP_MULT_PARAM]].value;
        multiple = opMultipleKnob[op] + (int)modMatrix[op].getDestinationValue(RATIO_DEST);

        pOpFreqs[0] = freqLUT.getFrequency((opPitch[op]) + bPitch) * multiples[clamp(multiple, 0, 26)];
        if(pOpFreqs[0] >= 22049.f) {
            pOpFreqs[0] = 22049.f;
        }
        __lowChordMask = __lowChordMaskTable[chordNotes.size() - 1];
        __highChordMask = __highChordMaskTable[chordNotes.size() - 1];
        __lowLeftGain = __lowLeftGainTable[chordNotes.size() - 1];
        __lowRightGain = __lowRightGainTable[chordNotes.size() - 1];
        __highLeftGain = __highLeftGainTable[chordNotes.size() - 1];
        __highRightGain = __highRightGainTable[chordNotes.size() - 1];

        for(unsigned long i = 0; i < chordNotes.size(); ++i) {
            pOpFreqs[i + 1] = freqLUT.getFrequency((opPitch[op]) + aPitch + chordNotes[i]) * multiples[clamp(multiple, 0, 26)];
            if(pOpFreqs[i + 1] >= 22049.f) {
                pOpFreqs[i + 1] = 22049.f;
            }
            if(opButtonLFO[op] > 0.f || masterLFO > 0.f) {
                pOpFreqs[i + 1] /= 100.f;
            }
        }

        __opLowFreq[op] = _mm_load_ps(pOpFreqs);
        __opHighFreq[op] = _mm_load_ps(pOpFreqs + 4);

        opWaveBankKnob[op] = (int)params[opParams[op][OP_BANK_PARAM]].value;
        opWaveBank[op] = opWaveBankKnob[op] + modMatrix[op].getDestinationValue(WAVE_BANK_DEST);
        opWaveBank[op] = clamp(opWaveBank[op], 0, NUM_WAVEBANKS - 1);

        opWave[op] = params[opParams[op][OP_WAVE_PARAM]].value;
        opWave[op] += inputs[opCVInputs[op][OP_WAVE_CV_1]].value * 0.1f * params[opCVAtten[op][OP_WAVE_CV_1]].value;
        opWave[op] += inputs[opCVInputs[op][OP_WAVE_CV_2]].value * 0.1f * params[opCVAtten[op][OP_WAVE_CV_2]].value;
        opWave[op] += modMatrix[op].getDestinationValue(WAVE_POS_DEST);
        opWave[op] = clamp(opWave[op], 0.f, (float)NUM_OP_WAVES);

        opShape[op] = params[opParams[op][OP_SHAPE_PARAM]].value + allShape;
        opShape[op] += inputs[opCVInputs[op][OP_SHAPE_CV_1]].value * 0.1f * params[opCVAtten[op][OP_SHAPE_CV_1]].value;
        opShape[op] += inputs[opCVInputs[op][OP_SHAPE_CV_2]].value * 0.1f * params[opCVAtten[op][OP_SHAPE_CV_2]].value;
        opShape[op] += modMatrix[op].getDestinationValue(SHAPE_DEST);
        opShape[op] = clamp(opShape[op], -1.f, 1.f);

        opLevel[op] = params[opParams[op][OP_LEVEL_PARAM]].value;
        opLevel[op] += inputs[opCVInputs[op][OP_LEVEL_CV_1]].value * 0.1f * params[opCVAtten[op][OP_LEVEL_CV_1]].value;
        opLevel[op] += inputs[opCVInputs[op][OP_LEVEL_CV_2]].value * 0.1f * params[opCVAtten[op][OP_LEVEL_CV_2]].value;
        opLevel[op] += modMatrix[op].getDestinationValue(LEVEL_DEST);
        opLevel[op] = clamp(opLevel[op], 0.f, 1.0f);

        coreA._mm_setFrequency(op, __opLowFreq[op]);
        coreB._mm_setFrequency(op, __opHighFreq[op]);
        coreA.setWavebank(op, opWaveBank[op]);
        coreB.setWavebank(op, opWaveBank[op]);
        coreA.setWavePosition(op, opWave[op]);
        coreB.setWavePosition(op, opWave[op]);
        opShapeMode[op] = opMenuShapeMode[op] + modMatrix[op].getDestinationValue(SHAPE_MODE_DEST);
        opShapeMode[op] = clamp(opShapeMode[op], 0, kNumShapeModes - 1);
        coreA.setShapeMode(op, opShapeMode[op]);
        coreB.setShapeMode(op, opShapeMode[op]);
        coreA.setShape(op, opShape[op]);
        coreB.setShape(op, opShape[op]);
        coreA.setPMPostShape(op, opPostShape[op]);
        coreB.setPMPostShape(op, opPostShape[op]);
        if(opButtonPostShape[op] + modMatrix[op].getDestinationValue(POST_SHAPE_DEST) > 0.5f) {
            opPostShape[op] = true;
        }
        else {
            opPostShape[op] = false;
        }
        coreA.enableSync(op, opSync[op]);
        coreB.enableSync(op, opSync[op]);

        coreA.externalFM(op, modMatrix[op].getDestinationValue(EXT_FM_DEST));
        coreB.externalFM(op, modMatrix[op].getDestinationValue(EXT_FM_DEST));
        coreA.externalSync(op, modMatrix[op].getDestinationValue(EXT_SYNC_DEST));
        coreB.externalSync(op, modMatrix[op].getDestinationValue(EXT_SYNC_DEST));

        opSyncMode[op] = opMenuSyncMode[op] + modMatrix[op].getDestinationValue(SYNC_MODE_DEST);
        opSyncMode[op] = clamp(opSyncMode[op], 0, kNumSyncModes - 1);
        coreA.setSyncMode(op, opSyncMode[op]);
        coreB.setSyncMode(op, opSyncMode[op]);

        // Enable sync if any external mod jack is connected, regardless of the sync button state
        bool modJackState = inputs[opCVInputs[op][OP_MOD_CV_1]].active | inputs[opCVInputs[op][OP_MOD_CV_2]].active
                          | inputs[opCVInputs[op][OP_MOD_CV_3]].active | inputs[opCVInputs[op][OP_MOD_CV_4]].active;
        coreA.enableSync(op, modJackState);
        coreB.enableSync(op, modJackState);

        // Internal sync enable
        if((opButtonSync[op] + modMatrix[op].getDestinationValue(SYNC_ENABLE_DEST) > 0.5f)) {
            opSync[op] = true;
        }
        else {
            opSync[op] = false;
        }
        coreA.enableIntSync(op, opSync[op]);
        coreB.enableIntSync(op, opSync[op]);

        if(opButtonWeakSync[op] + modMatrix[op].getDestinationValue(WEAK_SYNC_DEST) > 0.f) {
            opWeakSync[op] = true;
        }
        else {
            opWeakSync[op] = false;
        }
        coreA.enableWeakSync(op, opWeakSync[op]);
        coreB.enableWeakSync(op, opWeakSync[op]);
        if(opButtonPreFade[op] > 0.f) {
            coreA.setOpPreFade(op, true);
        }
        else {
            coreA.setOpPreFade(op, false);
        }
        coreA.setLevel(op, opLevel[op]);
        coreB.setLevel(op, opLevel[op]);
    }
    algo = round_int(params[ALGORITHM_PARAM].value + inputs[ALGO_INPUT].value * 0.1f * params[ALGO_DEPTH_PARAM].value * (float)(kNumAlgorithms - 1.f));
    algo = clamp(algo, 0, kNumAlgorithms - 1);
    coreA.setAlgorithm(algo);
    coreB.setAlgorithm(algo);
    feedback = params[FEEDBACK_PARAM].value;
    feedback += inputs[FEEDBACK_INPUT].value * 0.025 * params[FB_DEPTH_PARAM].value;
    feedback = clamp(feedback, 0.f, 0.25f);
    coreA.setFeedback(feedback);
    coreB.setFeedback(feedback);
    coreA.setBrightness(brightness);
    coreB.setBrightness(brightness);

    coreA.process();
    if(chordNotes.size() > 3) {
        coreB.process();
    }

    _mm_store_ps(pBOut, _mm_and_ps(coreA.getBOutput(), __bOutMask));
    outputs[B_OUTPUT].value = pBOut[0];

    if(outputs[A_RIGHT_OUTPUT].active) {
        __leftOut = __zeros;
        __rightOut = __zeros;
        __leftOut = _mm_mul_ps(coreA.getMainOutput(), __lowLeftGain);
        __leftOut = _mm_add_ps(__leftOut, _mm_mul_ps(coreB.getMainOutput(), __highLeftGain));
        __rightOut = _mm_mul_ps(coreA.getMainOutput(), __lowRightGain);
        __rightOut = _mm_add_ps(__rightOut, _mm_mul_ps(coreB.getMainOutput(), __highRightGain));
        _mm_store_ps(pLeftOut, __leftOut);
        _mm_store_ps(pRightOut, __rightOut);
    }
    else {
        _mm_store_ps(pLeftOut, _mm_add_ps(_mm_and_ps(coreA.getMainOutput(), __lowChordMask),
                                          _mm_and_ps(coreB.getMainOutput(), __highChordMask)));
        _mm_store_ps(pRightOut, __zeros);
    }

    if(indivBOutputs) {
        _mm_store_ps(pOP1, _mm_and_ps(coreA.getOpOutput(0), __0001));
        _mm_store_ps(pOP2, _mm_and_ps(coreA.getOpOutput(1), __0001));
        _mm_store_ps(pOP3, _mm_and_ps(coreA.getOpOutput(2), __0001));
        _mm_store_ps(pOP4, _mm_and_ps(coreA.getOpOutput(3), __0001));
    }
    else {
        _mm_store_ps(pOP1, _mm_add_ps(_mm_and_ps(coreA.getOpOutput(0), __lowChordMask),
                                      _mm_and_ps(coreB.getOpOutput(0), __highChordMask)));
        _mm_store_ps(pOP2, _mm_add_ps(_mm_and_ps(coreA.getOpOutput(1), __lowChordMask),
                                      _mm_and_ps(coreB.getOpOutput(1), __highChordMask)));
        _mm_store_ps(pOP3, _mm_add_ps(_mm_and_ps(coreA.getOpOutput(2), __lowChordMask),
                                      _mm_and_ps(coreB.getOpOutput(2), __highChordMask)));
        _mm_store_ps(pOP4, _mm_add_ps(_mm_and_ps(coreA.getOpOutput(3), __lowChordMask),
                                      _mm_and_ps(coreB.getOpOutput(3), __highChordMask)));
    }

    outputs[A_LEFT_OUTPUT].value = 0;
    outputs[A_RIGHT_OUTPUT].value = 0;
    outputs[OP_1_OUTPUT].value = 0;
    outputs[OP_2_OUTPUT].value = 0;
    outputs[OP_3_OUTPUT].value = 0;
    outputs[OP_4_OUTPUT].value = 0;
    for(auto i = 0; i < 4; ++i) {
        outputs[A_LEFT_OUTPUT].value += pLeftOut[i];
        outputs[A_RIGHT_OUTPUT].value += pRightOut[i];
        outputs[OP_1_OUTPUT].value += pOP1[i];
        outputs[OP_2_OUTPUT].value += pOP2[i];
        outputs[OP_3_OUTPUT].value += pOP3[i];
        outputs[OP_4_OUTPUT].value += pOP4[i];
    }
    outputs[A_LEFT_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
    outputs[A_RIGHT_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
    outputs[OP_1_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
    outputs[OP_2_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
    outputs[OP_3_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
    outputs[OP_4_OUTPUT].value *= _outputLevels[chordNotes.size() - 1];
}

void Dexter::makeChord(float chord, float invert) {
    currentChord = (Chords)((int)chord);
    invDepth = invert;
    chordNotes = getChord(currentChord, chordDetune, invDepth, fullInversion);
}

void Dexter::onSampleRateChange() {
    for(auto i = 0; i < kMaxChordSize; ++i) {
        coreA.setSampleRate(engineGetSampleRate());
        coreB.setSampleRate(engineGetSampleRate());
    }
}

void Dexter::reset() {
    for(auto i = 0; i < kNumOperators; ++i) {
        opMenuSyncMode[i] = 0;
        opMenuShapeMode[i] = 0;
        opMod1Assign[i] = 0;
        opMod2Assign[i] = 0;
        opMod3Assign[i] = 0;
        opMod4Assign[i] = 0;
    }
}

json_t *Dexter::toJson() {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "op1syncEnable", json_integer(opButtonSync[0]));
    json_object_set_new(rootJ, "op1weakSyncEnable", json_integer(opButtonWeakSync[0]));
    json_object_set_new(rootJ, "op1PostShape", json_integer(opButtonPostShape[0]));
    json_object_set_new(rootJ, "op1PreFade", json_integer(opButtonPreFade[0]));
    json_object_set_new(rootJ, "op1LFO", json_integer(opButtonLFO[0]));
    json_object_set_new(rootJ, "op1syncMode", json_integer(opMenuSyncMode[0]));
    json_object_set_new(rootJ, "op1shapeMode", json_integer(opMenuShapeMode[0]));
    json_object_set_new(rootJ, "op1Mod1Dest", json_integer(opMod1Assign[0]));
    json_object_set_new(rootJ, "op1Mod2Dest", json_integer(opMod2Assign[0]));
    json_object_set_new(rootJ, "op1Mod3Dest", json_integer(opMod3Assign[0]));
    json_object_set_new(rootJ, "op1Mod4Dest", json_integer(opMod4Assign[0]));

    json_object_set_new(rootJ, "op2syncEnable", json_integer(opButtonSync[1]));
    json_object_set_new(rootJ, "op2weakSyncEnable", json_integer(opButtonWeakSync[1]));
    json_object_set_new(rootJ, "op2PostShape", json_integer(opButtonPostShape[1]));
    json_object_set_new(rootJ, "op2PreFade", json_integer(opButtonPreFade[1]));
    json_object_set_new(rootJ, "op2LFO", json_integer(opButtonLFO[1]));
    json_object_set_new(rootJ, "op2syncMode", json_integer(opMenuSyncMode[1]));
    json_object_set_new(rootJ, "op2shapeMode", json_integer(opMenuShapeMode[1]));
    json_object_set_new(rootJ, "op2Mod1Dest", json_integer(opMod1Assign[1]));
    json_object_set_new(rootJ, "op2Mod2Dest", json_integer(opMod2Assign[1]));
    json_object_set_new(rootJ, "op2Mod3Dest", json_integer(opMod3Assign[1]));
    json_object_set_new(rootJ, "op2Mod4Dest", json_integer(opMod4Assign[1]));

    json_object_set_new(rootJ, "op3syncEnable", json_integer(opButtonSync[2]));
    json_object_set_new(rootJ, "op3weakSyncEnable", json_integer(opButtonWeakSync[2]));
    json_object_set_new(rootJ, "op3PostShape", json_integer(opButtonPostShape[2]));
    json_object_set_new(rootJ, "op3PreFade", json_integer(opButtonPreFade[2]));
    json_object_set_new(rootJ, "op3LFO", json_integer(opButtonLFO[2]));
    json_object_set_new(rootJ, "op3syncMode", json_integer(opMenuSyncMode[2]));
    json_object_set_new(rootJ, "op3shapeMode", json_integer(opMenuShapeMode[2]));
    json_object_set_new(rootJ, "op3Mod1Dest", json_integer(opMod1Assign[2]));
    json_object_set_new(rootJ, "op3Mod2Dest", json_integer(opMod2Assign[2]));
    json_object_set_new(rootJ, "op3Mod3Dest", json_integer(opMod3Assign[2]));
    json_object_set_new(rootJ, "op3Mod4Dest", json_integer(opMod4Assign[2]));

    json_object_set_new(rootJ, "op4syncEnable", json_integer(opButtonSync[3]));
    json_object_set_new(rootJ, "op4weakSyncEnable", json_integer(opButtonWeakSync[3]));
    json_object_set_new(rootJ, "op4PostShape", json_integer(opButtonPostShape[3]));
    json_object_set_new(rootJ, "op4PreFade", json_integer(opButtonPreFade[3]));
    json_object_set_new(rootJ, "op4LFO", json_integer(opButtonLFO[3]));
    json_object_set_new(rootJ, "op4syncMode", json_integer(opMenuSyncMode[3]));
    json_object_set_new(rootJ, "op4shapeMode", json_integer(opMenuShapeMode[3]));
    json_object_set_new(rootJ, "op4Mod1Dest", json_integer(opMod1Assign[3]));
    json_object_set_new(rootJ, "op4Mod2Dest", json_integer(opMod2Assign[3]));
    json_object_set_new(rootJ, "op4Mod3Dest", json_integer(opMod3Assign[3]));
    json_object_set_new(rootJ, "op4Mod4Dest", json_integer(opMod4Assign[3]));

    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));
    json_object_set_new(rootJ, "opSyncSource", json_integer((int)opSyncSource));
    json_object_set_new(rootJ, "masterLFO", json_integer(masterLFO));
    json_object_set_new(rootJ, "fullInversion", json_integer(buttonFullInversion));
    json_object_set_new(rootJ, "indivOutputSource", json_integer(indivBOutputs));

    return rootJ;
}

void Dexter::fromJson(json_t *rootJ) {
    json_t *j_opSyncEnable = json_object_get(rootJ, "op1syncEnable");
    json_t *j_opWeakSync = json_object_get(rootJ, "op1weakSyncEnable");
    json_t *j_opPostShape = json_object_get(rootJ, "op1PostShape");
    json_t *j_opPreFade = json_object_get(rootJ, "op1PreFade");
    json_t *j_opLFO = json_object_get(rootJ, "op1LFO");
    json_t *j_opSyncMode = json_object_get(rootJ, "op1syncMode");
    json_t *j_opShapeMode = json_object_get(rootJ, "op1shapeMode");
    json_t *j_opMod1Assign = json_object_get(rootJ, "op1Mod1Dest");
    json_t *j_opMod2Assign = json_object_get(rootJ, "op2Mod1Dest");
    json_t *j_opMod3Assign = json_object_get(rootJ, "op3Mod1Dest");
    json_t *j_opMod4Assign = json_object_get(rootJ, "op4Mod1Dest");

    opButtonSync[0] = json_integer_value(j_opSyncEnable);
    opButtonWeakSync[0] = json_integer_value(j_opWeakSync);
    opButtonPostShape[0] = json_integer_value(j_opPostShape);
    opButtonPreFade[0] = json_integer_value(j_opPreFade);
    opButtonLFO[0] = json_integer_value(j_opLFO);
    opMenuSyncMode[0] = json_integer_value(j_opSyncMode);
    opMenuShapeMode[0] = json_integer_value(j_opShapeMode);
    opMod1Assign[0] = json_integer_value(j_opMod1Assign);
    opMod2Assign[0] = json_integer_value(j_opMod2Assign);
    opMod3Assign[0] = json_integer_value(j_opMod3Assign);
    opMod4Assign[0] = json_integer_value(j_opMod4Assign);

    j_opSyncEnable = json_object_get(rootJ, "op2syncEnable");
    j_opWeakSync = json_object_get(rootJ, "op2weakSyncEnable");
    j_opPostShape = json_object_get(rootJ, "op2PostShape");
    j_opPreFade = json_object_get(rootJ, "op2PreFade");
    j_opLFO = json_object_get(rootJ, "op2LFO");
    j_opSyncMode = json_object_get(rootJ, "op2syncMode");
    j_opShapeMode = json_object_get(rootJ, "op2shapeMode");
    j_opMod1Assign = json_object_get(rootJ, "op2Mod1Dest");
    j_opMod2Assign = json_object_get(rootJ, "op2Mod2Dest");
    j_opMod3Assign = json_object_get(rootJ, "op2Mod3Dest");
    j_opMod4Assign = json_object_get(rootJ, "op2Mod4Dest");
    opButtonSync[1] = json_integer_value(j_opSyncEnable);
    opButtonWeakSync[1] = json_integer_value(j_opWeakSync);
    opButtonPostShape[1] = json_integer_value(j_opPostShape);
    opButtonPreFade[1] = json_integer_value(j_opPreFade);
    opButtonLFO[1] = json_integer_value(j_opLFO);
    opMenuSyncMode[1] = json_integer_value(j_opSyncMode);
    opMenuShapeMode[1] = json_integer_value(j_opShapeMode);
    opMod1Assign[1] = json_integer_value(j_opMod1Assign);
    opMod2Assign[1] = json_integer_value(j_opMod2Assign);
    opMod3Assign[1] = json_integer_value(j_opMod3Assign);
    opMod4Assign[1] = json_integer_value(j_opMod4Assign);

    j_opSyncEnable = json_object_get(rootJ, "op3syncEnable");
    j_opWeakSync = json_object_get(rootJ, "op3weakSyncEnable");
    j_opPostShape = json_object_get(rootJ, "op3PostShape");
    j_opPreFade = json_object_get(rootJ, "op3PreFade");
    j_opLFO = json_object_get(rootJ, "op3LFO");
    j_opSyncMode = json_object_get(rootJ, "op3syncMode");
    j_opShapeMode = json_object_get(rootJ, "op3shapeMode");
    j_opMod1Assign = json_object_get(rootJ, "op3Mod1Dest");
    j_opMod2Assign = json_object_get(rootJ, "op3Mod2Dest");
    j_opMod3Assign = json_object_get(rootJ, "op3Mod3Dest");
    j_opMod4Assign = json_object_get(rootJ, "op3Mod4Dest");
    opButtonSync[2] = json_integer_value(j_opSyncEnable);
    opButtonWeakSync[2] = json_integer_value(j_opWeakSync);
    opButtonPostShape[2] = json_integer_value(j_opPostShape);
    opButtonPreFade[2] = json_integer_value(j_opPreFade);
    opButtonLFO[2] = json_integer_value(j_opLFO);
    opMenuSyncMode[2] = json_integer_value(j_opSyncMode);
    opMenuShapeMode[2] = json_integer_value(j_opShapeMode);
    opMod1Assign[2] = json_integer_value(j_opMod1Assign);
    opMod2Assign[2] = json_integer_value(j_opMod2Assign);
    opMod3Assign[2] = json_integer_value(j_opMod3Assign);
    opMod4Assign[2] = json_integer_value(j_opMod4Assign);

    j_opSyncEnable = json_object_get(rootJ, "op4syncEnable");
    j_opWeakSync = json_object_get(rootJ, "op4weakSyncEnable");
    j_opPostShape = json_object_get(rootJ, "op4PostShape");
    j_opPreFade = json_object_get(rootJ, "op4PreFade");
    j_opLFO = json_object_get(rootJ, "op4LFO");
    j_opSyncMode = json_object_get(rootJ, "op4syncMode");
    j_opShapeMode = json_object_get(rootJ, "op4shapeMode");
    j_opMod1Assign = json_object_get(rootJ, "op4Mod1Dest");
    j_opMod2Assign = json_object_get(rootJ, "op4Mod2Dest");
    j_opMod3Assign = json_object_get(rootJ, "op4Mod3Dest");
    j_opMod4Assign = json_object_get(rootJ, "op4Mod4Dest");
    opButtonSync[3] = json_integer_value(j_opSyncEnable);
    opButtonWeakSync[3] = json_integer_value(j_opWeakSync);
    opButtonPostShape[3] = json_integer_value(j_opPostShape);
    opButtonPreFade[3] = json_integer_value(j_opPreFade);
    opButtonLFO[3] = json_integer_value(j_opLFO);
    opMenuSyncMode[3] = json_integer_value(j_opSyncMode);
    opMenuShapeMode[3] = json_integer_value(j_opShapeMode);
    opMod1Assign[3] = json_integer_value(j_opMod1Assign);
    opMod2Assign[3] = json_integer_value(j_opMod2Assign);
    opMod3Assign[3] = json_integer_value(j_opMod3Assign);
    opMod4Assign[3] = json_integer_value(j_opMod4Assign);

    json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
    panelStyle = json_integer_value(j_panelStyle);

    json_t *j_opSyncToParent = json_object_get(rootJ, "opSyncSource");
    opSyncSource = (FourVoiceOPCore::OpSyncSource)json_integer_value(j_opSyncToParent);

    json_t *j_masterLFO = json_object_get(rootJ, "masterLFO");
    masterLFO = json_integer_value(j_masterLFO);

    json_t *j_fullInversion = json_object_get(rootJ, "fullInversion");
    buttonFullInversion = json_integer_value(j_fullInversion);

    json_t *j_indivOutputSource = json_object_get(rootJ, "indivOutputSource");
    indivBOutputs = json_integer_value(j_indivOutputSource);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// UI ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

DexterWidget::DexterWidget(Dexter *module) : ModuleWidget(module) {
    {
        DynamicPanelWidget *panel = new DynamicPanelWidget();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/DexterPanelDark.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/DexterPanelLight.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
    }
    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    {
        AlgoGraphic* algo = new AlgoGraphic;
        algo->box.pos = Vec(138.75, 88.245);
        algo->value = &module->algo;
        algo->style = &module->panelStyle;
        addChild(algo);
    }

    addParam(createDynamicKnob<DynRoganMedBlue>(OctaveAKnobPos, module, Dexter::OCTAVE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, 0.0, 6.0, 3.0, SNAP_MOTION));
    addParam(createDynamicKnob<DynRoganMedBlue>(CoarseKnobPos, module, Dexter::COARSE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, -1.0, 1.0, 0.0, SMOOTH_MOTION));
    addParam(createDynamicKnob<DynRoganMedBlue>(FineKnobPos, module, Dexter::FINE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, -0.05, 0.05, 0.0, SMOOTH_MOTION));

    {
        DynamicKnob* dynParam = createDynamicKnob<DynRoganMedSmallBlue>(ChordKnobPos, module, Dexter::CHORD_PARAM, nullptr,
                                                  ACTIVE_LOW_VIEW, 0.0, NUM_CHORDS - 1, 0.0, SNAP_MOTION);
        addParam(dynParam);
    }

    {
        DynamicKnob* dynParam = createDynamicKnob<DynRoganMedSmallBlue>(InvertKnobPos, module, Dexter::INVERT_PARAM, nullptr,
                                                  ACTIVE_LOW_VIEW, 0, 20, 10, SNAP_MOTION);
        addParam(dynParam);
    }
    //addParam(ParamWidget::create<RoganMedBlue>(DetuneKnobPos, module, Dexter::DETUNE_PARAM, 0.0, 1.0, 0.0));

    {
        DynamicKnob* dynParam = createDynamicKnob<DynRoganMedSmallBlue>(DetuneKnobPos, module, Dexter::DETUNE_PARAM, nullptr,
                                                  ACTIVE_LOW_VIEW, 0.0, 1.0, 0.0, SMOOTH_MOTION);
        addParam(dynParam);
    }

    addParam(createDynamicKnob<DynRoganMedBlue>(Octave2KnobPos, module, Dexter::B_OCTAVE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, 0.0, 6.0, 3.0, SNAP_MOTION));
    addParam(createDynamicKnob<DynRoganMedBlue>(Coarse2KnobPos, module, Dexter::B_COARSE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, -1.0, 1.0, 0.0, SMOOTH_MOTION));
    addParam(createDynamicKnob<DynRoganMedBlue>(Fine2KnobPos, module, Dexter::B_FINE_PARAM, nullptr,
                                                ACTIVE_LOW_VIEW, -0.05, 0.05, 0.0, SMOOTH_MOTION));

    addParam(ParamWidget::create<RoganMedBlueSnap>(AlgoKnobPos, module, Dexter::ALGORITHM_PARAM, 0, 22, 0.0));
    addParam(ParamWidget::create<RoganMedGreen>(BrightKnobPos, module, Dexter::BRIGHTNESS_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganMedRed>(ShapeKnobPos, module, Dexter::SHAPE_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganMedOrange>(FeedbackKnobPos, module, Dexter::FEEDBACK_PARAM, 0.0, 0.25, 0.0));

    addParam(ParamWidget::create<RoganSmallBlue>(ChordDepthKnobPos, module, Dexter::CHORD_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallBlue>(InvertDepthKnobPos, module, Dexter::INVERT_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallBlue>(DetuneDepthKnobPos, module, Dexter::DETUNE_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallBlue>(AlgoDepthKnobPos, module, Dexter::ALGO_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallOrange>(FBDepthKnobPos, module, Dexter::FB_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallGreen>(BrightDepthKnobPos, module, Dexter::BRIGHT_DEPTH_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<RoganSmallRed>(ShapeDepthKnobPos, module, Dexter::SHAPE_DEPTH_PARAM, -1.0, 1.0, 0.0));

    addParam(ParamWidget::create<LightLEDButton>(MasterLFOButtonPos, module, Dexter::MASTER_LFO_BUTTON, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<LightLEDButton>(ResetPhaseButtonPos, module, Dexter::RESET_PHASE_BUTTON, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<LightLEDButton>(FullInversionButtonPos, module, Dexter::FULL_INVERSION_BUTTON, 0.0, 1.0, 0.0));

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(MasterLFOButtonLEDPos, module, Dexter::MASTER_LFO_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(ResetPhaseButtonLEDPos, module, Dexter::RESET_PHASE_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(FullInversionButtonLEDPos, module, Dexter::FULL_INVERSION_LIGHT));

    addOutput(Port::create<PJ301MDarkSmallOut>(AOutLeftJack, Port::OUTPUT, module, Dexter::A_LEFT_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(AOutRightJack, Port::OUTPUT, module, Dexter::A_RIGHT_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(Op1OutJack, Port::OUTPUT, module, Dexter::OP_1_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(Op2OutJack, Port::OUTPUT, module, Dexter::OP_2_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(Op3OutJack, Port::OUTPUT, module, Dexter::OP_3_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(Op4OutJack, Port::OUTPUT, module, Dexter::OP_4_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(BOutJack, Port::OUTPUT, module, Dexter::B_OUTPUT));
    addInput(Port::create<PJ301MDarkSmall>(VOct1CVJack, Port::INPUT, module, Dexter::A_VOCT_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(VOct2CVJack, Port::INPUT, module, Dexter::B_VOCT_INPUT));

    addInput(Port::create<PJ301MDarkSmall>(ChordCVJack, Port::INPUT, module, Dexter::CHORD_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(InvertCVJack, Port::INPUT, module, Dexter::INVERT_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(DetuneCVJack, Port::INPUT, module, Dexter::DETUNE_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(AlgorithmCVJack, Port::INPUT, module, Dexter::ALGO_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(FeedbackCVJack, Port::INPUT, module, Dexter::FEEDBACK_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(BrightCVJack, Port::INPUT, module, Dexter::BRIGHTNESS_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(ShapeCVJack, Port::INPUT, module, Dexter::SHAPE_INPUT));
    {
        DynamicFrameText* chordText = new DynamicFrameText;
        chordText->size = 12;
        chordText->box.pos = Vec(24.91, 124.468);
        chordText->box.size = Vec(82, 14);
        chordText->itemHandle = &module->chordKnob;
        chordText->visibility = nullptr;
        chordText->viewMode = ACTIVE_LOW_VIEW;
        chordText->colorHandle = &module->panelStyle;
        for(auto i = 0; i < NUM_CHORDS; ++i) {
            chordText->addItem(chordNames[i]);
        }
        addChild(chordText);
    }

    {
        DynamicFrameText* octaveText = new DynamicFrameText;
        octaveText->size = 14;
        octaveText->box.pos = Vec(35.61, 68.076);
        octaveText->box.size = Vec(82, 14);
        octaveText->itemHandle = &module->octaveAKnob;
        octaveText->visibility = nullptr;
        octaveText->viewMode = ACTIVE_LOW_VIEW;
        octaveText->colorHandle = &module->panelStyle;
        octaveText->addItem("-3");
        octaveText->addItem("-2");
        octaveText->addItem("-1");
        octaveText->addItem("0");
        octaveText->addItem("+1");
        octaveText->addItem("+2");
        octaveText->addItem("+3");
        addChild(octaveText);
    }

    {
        DynamicFrameText* octaveText = new DynamicFrameText;
        octaveText->size = 14;
        octaveText->box.pos = Vec(35.61, 186.076);
        octaveText->box.size = Vec(82, 14);
        octaveText->itemHandle = &module->octaveBKnob;
        octaveText->visibility = nullptr;
        octaveText->viewMode = ACTIVE_LOW_VIEW;
        octaveText->colorHandle = &module->panelStyle;
        octaveText->addItem("-3");
        octaveText->addItem("-2");
        octaveText->addItem("-1");
        octaveText->addItem("0");
        octaveText->addItem("+1");
        octaveText->addItem("+2");
        octaveText->addItem("+3");
        addChild(octaveText);
    }

    // Operator Controls
    float operatorSpacing = 125.5275;
    float offset = 0.0;
    for(auto op = 0; op < kNumOperators; ++op) {
        offset = operatorSpacing * op;

        addParam(createDynamicKnob<DynRoganMedBlue>(Vec(OpMultKnobRootX + offset, OpRow1Y), module,
                                                    module->opParams[op][Dexter::OP_MULT_PARAM], &module->opSettingsMenu[op],
                                                    ACTIVE_LOW_VIEW, 0, 26, 3, SNAP_MOTION));
        {
            DynamicFrameText* multText = new DynamicFrameText;
            multText->size = 14;
            multText->box.pos = Vec(OpMainTextX[0] + offset + 19, OpMainTextY[0] - 2.718);
            multText->box.size = Vec(81, 14);
            multText->itemHandle = &module->opMultipleKnob[op];
            multText->visibility = &module->opSettingsMenu[op];
            multText->viewMode = ACTIVE_LOW_VIEW;
            multText->colorHandle = &module->panelStyle;
            for(unsigned long i = 0; i < kNumMultiples; ++i) {
                multText->addItem(module->multiplesText[i]);
            }
            addChild(multText);
        }
        addParam(createDynamicKnob<DynRoganMedBlue>(Vec(OpCoarseKnobRootX + offset, OpRow1Y), module,
                                                    module->opParams[op][Dexter::OP_COARSE_PARAM], &module->opSettingsMenu[op],
                                                    ACTIVE_LOW_VIEW, -1.0, 1.0, 0.0, SMOOTH_MOTION));
        addParam(createDynamicKnob<DynRoganMedBlue>(Vec(OpFineKnobRootX + offset, OpRow1Y), module,
                                                    module->opParams[op][Dexter::OP_FINE_PARAM], &module->opSettingsMenu[op],
                                                    ACTIVE_LOW_VIEW, -0.25, 0.25, 0.0, SMOOTH_MOTION));
        addParam(createDynamicKnob<DynRoganMedPurple>(Vec(OpWaveKnobRootX + offset, OpRow2Y), module,
                                                    module->opParams[op][Dexter::OP_WAVE_PARAM], &module->opSettingsMenu[op],
                                                    ACTIVE_LOW_VIEW, 0.0, 1.0, 0.0, SMOOTH_MOTION));
        addParam(createDynamicKnob<DynRoganMedRed>(Vec(OpShapeKnobRootX + offset, OpRow2Y), module,
                                                    module->opParams[op][Dexter::OP_SHAPE_PARAM], &module->opSettingsMenu[op],
                                                    ACTIVE_LOW_VIEW, 0.0, 1.0, 0.0, SMOOTH_MOTION));
        if(op == 0) {
            addParam(createDynamicKnob<DynRoganMedGreen>(Vec(OpLevelKnobRootX + offset, OpRow2Y), module,
                                                        module->opParams[op][Dexter::OP_LEVEL_PARAM], &module->opSettingsMenu[op],
                                                        ACTIVE_LOW_VIEW, 0.0, 1.0, 1.0, SMOOTH_MOTION));
        }
        else {
            addParam(createDynamicKnob<DynRoganMedGreen>(Vec(OpLevelKnobRootX + offset, OpRow2Y), module,
                                                        module->opParams[op][Dexter::OP_LEVEL_PARAM], &module->opSettingsMenu[op],
                                                        ACTIVE_LOW_VIEW, 0.0, 1.0, 0.0, SMOOTH_MOTION));
        }

        // Buttons
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpPreButtonPosX + offset, OpPreButtonPosY), module,
                                             module->opParams[op][Dexter::OP_PRE_PARAM], 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpSettingsButtonRootX + offset, OpSettingsButtonRootY),
                                             module, module->opParams[op][Dexter::OP_SETTINGS_PARAM], 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpPercButtonRootX + offset, OpPercButtonRootY), module,
                                             module->opParams[op][Dexter::OP_POST_SHAPE_PARAM], 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpIsolateButtonRootX + offset, OpIsolateButtonRootY), module,
                                             module->opParams[op][Dexter::OP_WEAK_PARAM], 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpLFOButtonRootX + offset, OpLFOButtonRootY), module,
                                             module->opParams[op][Dexter::OP_LFO_PARAM], 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LightLEDButton>(Vec(OpSyncButtonRootX + offset, OpSyncButtonRootY), module,
                                             module->opParams[op][Dexter::OP_SYNC_PARAM], 0.0, 1.0, 0.0));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpPreButtonPosX + offset + ledOffset,
                                                 OpPreButtonPosY + ledOffset), module, module->opLights[op][Dexter::OP_PRE_LIGHT]));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpSettingsButtonRootX + offset + ledOffset,
                                                 OpSettingsButtonRootY + ledOffset), module, module->opLights[op][Dexter::OP_SETTINGS]));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpPercButtonRootX + offset + ledOffset,
                                                 OpPercButtonRootY + ledOffset), module, module->opLights[op][Dexter::OP_POST_SHAPE]));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpIsolateButtonRootX + offset + ledOffset,
                                                 OpIsolateButtonRootY + ledOffset), module, module->opLights[op][Dexter::OP_WEAK]));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpLFOButtonRootX + offset + ledOffset,
                                                 OpLFOButtonRootY + ledOffset), module, module->opLights[op][Dexter::OP_LFO]));
        addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(OpSyncButtonRootX + offset + ledOffset,
                                                 OpSyncButtonRootY + ledOffset), module, module->opLights[op][Dexter::OP_SYNC]));

        addParam(createDynamicSwitchWidget<LightLEDButton>(Vec(OpWaveButtonX + offset, 110.375),
                                                        module, module->opParams[op][Dexter::OP_WAVE_MENU_PARAM],
                                                        0.0, 1.0, 0.0, &module->opSettingsMenu[op],
                                                        ACTIVE_HIGH_VIEW));
        addParam(createDynamicSwitchWidget<LightLEDButton>(Vec(OpModAButtonX + offset, 110.375),
                                                        module, module->opParams[op][Dexter::OP_MODA_MENU_PARAM],
                                                        0.0, 1.0, 0.0, &module->opSettingsMenu[op],
                                                        ACTIVE_HIGH_VIEW));
        addParam(createDynamicSwitchWidget<LightLEDButton>(Vec(OpModBButtonX + offset, 110.375),
                                                        module, module->opParams[op][Dexter::OP_MODB_MENU_PARAM],
                                                        0.0, 1.0, 0.0, &module->opSettingsMenu[op],
                                                        ACTIVE_HIGH_VIEW));

        addChild(createDynamicLight<MediumLight<RedDynamicLight>>(Vec(OpWaveButtonX + offset + ledOffset,
                                                    110.375 + ledOffset), module, module->opLights[op][Dexter::OP_WAVE_LIGHT],
                                                    &module->opSettingsMenu[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicLight<MediumLight<RedDynamicLight>>(Vec(OpModAButtonX + offset + ledOffset,
                                                    110.375 + ledOffset), module, module->opLights[op][Dexter::OP_MODA_LIGHT],
                                                    &module->opSettingsMenu[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicLight<MediumLight<RedDynamicLight>>(Vec(OpModBButtonX + offset + ledOffset,
                                                    110.375 + ledOffset), module, module->opLights[op][Dexter::OP_MODB_LIGHT],
                                                    &module->opSettingsMenu[op], ACTIVE_HIGH_VIEW));

        for(auto i = 0; i < 6; ++i){
            DynamicText* dynText = new DynamicText();
            dynText->size = 14;
            dynText->text = std::make_shared<std::string>(OpMainText[i]);
            dynText->box.pos = Vec(OpMainTextX[i] + offset, OpMainTextY[i] - 2.718);
            dynText->box.size = Vec(82,14);
            dynText->visibility = &module->opSettingsMenu[op];
            dynText->viewMode = ACTIVE_LOW_VIEW;
            dynText->colorHandle = &module->panelStyle;
            addChild(dynText);
        }

        // Settings Menu
        addChild(createDynamicText(Vec(203.2045 + offset, 125), 12, "Wave", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(203.2045 + offset, 134), 12, "Table", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));

        addChild(createDynamicText(Vec(234.6105 + offset, 125), 12, "Mod", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(234.6105 + offset, 134), 12, "1 & 2", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));

        addChild(createDynamicText(Vec(266.0295 + offset, 125), 12, "Mod", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(266.0295 + offset, 134), 12, "3 & 4", &module->opSettingsMenu[op], &module->panelStyle, ACTIVE_HIGH_VIEW));

        // Wavetable Menu
        addParam(createDynamicKnob<DynRoganMedBlue>(Vec(OpTableKnobRootX + offset, OpRow1Y), module,
                                                    module->opParams[op][Dexter::OP_BANK_PARAM], &module->opWaveMenuVis[op],
                                                    ACTIVE_HIGH_VIEW, 0.0, (float)(NUM_WAVEBANKS - 1.f), 0.0, SNAP_MOTION));

        std::vector<std::string> syncItems(syncModes, syncModes + kNumSyncModes);
        addChild(createDynamicChoice(Vec(238.819 + offset, 55.25), 67.806, syncItems, &module->opMenuSyncMode[op],
                 &module->opWaveMenuVis[op], ACTIVE_HIGH_VIEW));
        std::vector<std::string> shapeItems(shapeModes, shapeModes + kNumShapeModes);
        addChild(createDynamicChoice(Vec(238.819 + offset, 87.125), 67.806, shapeItems, &module->opMenuShapeMode[op],
                 &module->opWaveMenuVis[op], ACTIVE_HIGH_VIEW));

        addChild(createDynamicText(Vec(211.062 + offset, 81.468 - 2.718), 14, "Table", &module->opWaveMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(273.222 + offset, 45.058 - 4.0), 12, "Sync Mode", &module->opWaveMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(273.222 + offset, 77.933 - 4.0), 12, "Shape Mode", &module->opWaveMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));

        {
            DynamicFrameText* tableText = new DynamicFrameText;
            tableText->size = 12;
            tableText->box.pos = Vec(211.062 + offset, 91.468);
            tableText->box.size = Vec(82, 14);
            tableText->itemHandle = &module->opWaveBankKnob[op];
            tableText->visibility = &module->opWaveMenuVis[op];
            tableText->viewMode = ACTIVE_HIGH_VIEW;
            tableText->colorHandle = &module->panelStyle;
            for(auto i = 0; i < NUM_WAVEBANKS; ++i) {
                tableText->addItem(waveTableNames[i]);
            }
            addChild(tableText);
        }

        // Mod Menus
        std::vector<std::string> modAssignItems(modDest, modDest + NUM_DESTS);
        addChild(createDynamicChoice(Vec(226.f + offset, 55.25f), 80.f, modAssignItems, &module->opMod1Assign[op], &module->opModAMenuVis[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicChoice(Vec(226.f + offset, 87.125f), 80.f, modAssignItems, &module->opMod2Assign[op], &module->opModAMenuVis[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(206.f + offset, 61.058f - 4.f), 14, "Mod 1", &module->opModAMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(206.f + offset, 92.933f - 4.f), 14, "Mod 2", &module->opModAMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));

        addChild(createDynamicChoice(Vec(226.f + offset, 55.25f), 80.f, modAssignItems, &module->opMod3Assign[op], &module->opModBMenuVis[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicChoice(Vec(226.f + offset, 87.125f), 80.f, modAssignItems, &module->opMod4Assign[op], &module->opModBMenuVis[op], ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(206.f + offset, 61.058f - 4.f), 14, "Mod 3", &module->opModBMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));
        addChild(createDynamicText(Vec(206.f + offset, 92.933f - 4.f), 14, "Mod 4", &module->opModBMenuVis[op], &module->panelStyle, ACTIVE_HIGH_VIEW));


        addParam(ParamWidget::create<RoganSmallMustard>(Vec(OpMod1KnobRootX + offset, OpModRowY), module, module->opCVAtten[op][0], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallMustard>(Vec(OpMod2KnobRootX + offset, OpModRowY), module, module->opCVAtten[op][1], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallMustard>(Vec(OpMod3KnobRootX + offset, OpModRowY), module, module->opCVAtten[op][2], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallMustard>(Vec(OpMod4KnobRootX + offset, OpModRowY), module, module->opCVAtten[op][3], -1.0, 1.0, 0.0));

        addParam(ParamWidget::create<RoganSmallBlue>(Vec(OpPitch1KnobRootX + offset, OpCV1RowY), module, module->opCVAtten[op][4], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallPurple>(Vec(OpWave1KnobRootX + offset, OpCV1RowY), module, module->opCVAtten[op][5], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallRed>(Vec(OpShape1KnobRootX + offset, OpCV1RowY), module, module->opCVAtten[op][6], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallGreen>(Vec(OpLevel1KnobRootX + offset, OpCV1RowY), module, module->opCVAtten[op][7], -1.0, 1.0, 0.0));

        addParam(ParamWidget::create<RoganSmallBlue>(Vec(OpPitch2KnobRootX + offset, OpCV2RowY), module, module->opCVAtten[op][8], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallPurple>(Vec(OpWave2KnobRootX + offset, OpCV2RowY), module, module->opCVAtten[op][9], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallRed>(Vec(OpShape2KnobRootX + offset, OpCV2RowY), module, module->opCVAtten[op][10], -1.0, 1.0, 0.0));
        addParam(ParamWidget::create<RoganSmallGreen>(Vec(OpLevel2KnobRootX + offset, OpCV2RowY), module, module->opCVAtten[op][11], -1.0, 1.0, 0.0));

        //addInput(createInput<PJ301MDarkSmall>(Vec(OpSyncJackRootX + offset, OpSyncJackRootY), module, module->opSyncInputs[op]);
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV1JackRootX + offset, OpModJackRowY), Port::INPUT, module, module->opCVInputs[op][0]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV2JackRootX + offset, OpModJackRowY), Port::INPUT, module, module->opCVInputs[op][1]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV3JackRootX + offset, OpModJackRowY), Port::INPUT, module, module->opCVInputs[op][2]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV4JackRootX + offset, OpModJackRowY), Port::INPUT, module, module->opCVInputs[op][3]));

        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV1JackRootX + offset, OpCV1JackRootY), Port::INPUT, module, module->opCVInputs[op][4]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV2JackRootX + offset, OpCV1JackRootY), Port::INPUT, module, module->opCVInputs[op][5]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV3JackRootX + offset, OpCV1JackRootY), Port::INPUT, module, module->opCVInputs[op][6]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV4JackRootX + offset, OpCV1JackRootY), Port::INPUT, module, module->opCVInputs[op][7]));

        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV1JackRootX + offset, OpCV2JackRootY), Port::INPUT, module, module->opCVInputs[op][8]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV2JackRootX + offset, OpCV2JackRootY), Port::INPUT, module, module->opCVInputs[op][9]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV3JackRootX + offset, OpCV2JackRootY), Port::INPUT, module, module->opCVInputs[op][10]));
        addInput(Port::create<PJ301MDarkSmall>(Vec(OpCV4JackRootX + offset, OpCV2JackRootY), Port::INPUT, module, module->opCVInputs[op][11]));

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct DexterPanelStyleItem : MenuItem {
    Dexter* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

struct DexterOpSyncSourceItem : MenuItem {
    Dexter* module;
    FourVoiceOPCore::OpSyncSource opSyncSource = FourVoiceOPCore::PARENT_SYNC_SOURCE;
    void onAction(EventAction &e) override {
        module->opSyncSource = opSyncSource;
    }
    void step() override {
        rightText = (module->opSyncSource == opSyncSource) ? "✔" : "";
        MenuItem::step();
    }
};

struct DexterOpOuputSource : MenuItem {
    Dexter* module;
    unsigned long indivBOutputs = 0;
    void onAction(EventAction &e) override {
        module->indivBOutputs = indivBOutputs;
    }
    void step() override {
        rightText = (module->indivBOutputs == indivBOutputs) ? "✔" : "";
        MenuItem::step();
    }
};

void DexterWidget::appendContextMenu(Menu *menu) {
    Dexter *module = dynamic_cast<Dexter*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Panel Style"));
    menu->addChild(construct<DexterPanelStyleItem>(&MenuItem::text, "Dark", &DexterPanelStyleItem::module,
                                                   module, &DexterPanelStyleItem::panelStyle, 0));
    menu->addChild(construct<DexterPanelStyleItem>(&MenuItem::text, "Light", &DexterPanelStyleItem::module,
                                                   module, &DexterPanelStyleItem::panelStyle, 1));

    // Operator Sync Source
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Operator Sync Source"));
    menu->addChild(construct<DexterOpSyncSourceItem>(&MenuItem::text, "Parent", &DexterOpSyncSourceItem::module,
                                                     module, &DexterOpSyncSourceItem::opSyncSource, FourVoiceOPCore::PARENT_SYNC_SOURCE));
    menu->addChild(construct<DexterOpSyncSourceItem>(&MenuItem::text, "Neighbour", &DexterOpSyncSourceItem::module,
                                                     module, &DexterOpSyncSourceItem::opSyncSource, FourVoiceOPCore::NEIGHBOUR_SYNC_SOURCE));

     // Indiv output source
     menu->addChild(construct<MenuLabel>());
     menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Operator Outputs Source"));
     menu->addChild(construct<DexterOpOuputSource>(&MenuItem::text, "Voice A", &DexterOpOuputSource::module,
                                                   module, &DexterOpOuputSource::indivBOutputs, 0));
     menu->addChild(construct<DexterOpOuputSource>(&MenuItem::text, "Voice B", &DexterOpOuputSource::module,
                                                   module, &DexterOpOuputSource::indivBOutputs, 1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Valley, Dexter) {
   Model *modelDexter = Model::create<Dexter, DexterWidget>(TOSTRING(SLUG), "Dexter", "Dexter", OSCILLATOR_TAG);
   return modelDexter;
}
