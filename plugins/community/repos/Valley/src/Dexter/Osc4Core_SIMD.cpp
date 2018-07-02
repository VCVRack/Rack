#include "Osc4Core_SIMD.hpp"

FourVoiceOPCore::FourVoiceOPCore() {
    _algorithm = -1;
    setWavebank(0, 0);
    setWavebank(1, 0);
    setWavebank(2, 0);
    setWavebank(3, 0);
    setWavePosition(0, 0.f);
    setWavePosition(1, 0.f);
    setWavePosition(2, 0.f);
    setWavePosition(3, 0.f);
    __ones = _mm_set1_ps(1.f);
    __zeros = _mm_set1_ps(0.f);
    __five = _mm_set1_ps(5.f);
    __outputLevels[0] = _mm_set1_ps(5.f);
    __outputLevels[1] = _mm_set1_ps(3.3f);
    __outputLevels[2] = _mm_set1_ps(2.5f);
    __outputLevels[3] = _mm_set1_ps(2.f);
    _opPreFade[0] = false;
    _opPreFade[1] = false;
    _opPreFade[2] = false;
    _opPreFade[3] = false;
    __opLevel[0] = __zeros;
    __opLevel[1] = __zeros;
    __opLevel[2] = __zeros;
    __opLevel[3] = __zeros;
    __opOut[0] = __zeros;
    __opOut[1] = __zeros;
    __opOut[2] = __zeros;
    __opOut[3] = __zeros;
    __opAuxOut[0] = __zeros;
    __opAuxOut[1] = __zeros;
    __opAuxOut[2] = __zeros;
    __opAuxOut[3] = __zeros;
    __op1Eoc = __zeros;
    __op2Eoc = __zeros;
    __op3Eoc = __zeros;
    __op4Eoc = __zeros;
    __opExtFM[0] = __zeros;
    __opExtFM[1] = __zeros;
    __opExtFM[2] = __zeros;
    __opExtFM[3] = __zeros;
    __opExtSync[0] = __zeros;
    __opExtSync[1] = __zeros;
    __opExtSync[2] = __zeros;
    __opExtSync[3] = __zeros;
    __opSyncEnable[0] = __zeros;
    __opSyncEnable[1] = __zeros;
    __opSyncEnable[2] = __zeros;
    __opSyncEnable[3] = __zeros;
    __op1Col = __zeros;
    __op2Col = __zeros;
    __op3Col = __zeros;
    __op4Col = __zeros;
    setAlgorithm(0);
}

void FourVoiceOPCore::process() {
    calcOpLevels();
    mix();
    _op[3].tick();
    _op[2].tick();
    _op[1].tick();
    _op[0].tick();
}

void FourVoiceOPCore::resetPhase() {
    _op[0].resetPhase();
    _op[1].resetPhase();
    _op[2].resetPhase();
    _op[3].resetPhase();
}

__m128 FourVoiceOPCore::getMainOutput() const {
    return _mm_mul_ps(__mainCol, __aOutLevel);
}

__m128 FourVoiceOPCore::getBOutput() const {
    return _mm_mul_ps(__bCol, __bOutLevel);
}

__m128 FourVoiceOPCore::getOpOutput(int opNum) const {
    if(_opPreFade[opNum]) {
        return _mm_mul_ps(__opAuxOut[opNum], __five);
    }
    return _mm_mul_ps(__opOut[opNum], __five);
}

void FourVoiceOPCore::setAlgorithm(int newAlgorithm) {
    if(newAlgorithm != _algorithm) {
        _algorithm = newAlgorithm;
        clearMatrix();
        switch (_algorithm) {
            case 0: setMatrixAlgo0(); break;
            case 1: setMatrixAlgo1(); break;
            case 2: setMatrixAlgo2(); break;
            case 3: setMatrixAlgo3(); break;
            case 4: setMatrixAlgo4(); break;
            case 5: setMatrixAlgo5(); break;
            case 6: setMatrixAlgo6(); break;
            case 7: setMatrixAlgo7(); break;
            case 8: setMatrixAlgo8(); break;
            case 9: setMatrixAlgo9(); break;
            case 10: setMatrixAlgo10(); break;
            case 11: setMatrixAlgo11(); break;
            case 12: setMatrixAlgo12(); break;
            case 13: setMatrixAlgo13(); break;
            case 14: setMatrixAlgo14(); break;
            case 15: setMatrixAlgo15(); break;
            case 16: setMatrixAlgo16(); break;
            case 17: setMatrixAlgo17(); break;
            case 18: setMatrixAlgo18(); break;
            case 19: setMatrixAlgo19(); break;
            case 20: setMatrixAlgo20(); break;
            case 21: setMatrixAlgo21(); break;
            case 22: setMatrixAlgo22(); break;
            default: setMatrixAlgo22();
        }
    }
}

void FourVoiceOPCore::setFeedback(float feedback) {
    __matrix[OP_3_ROW][OP_3_COL] = _mm_set1_ps(feedback);
}

void FourVoiceOPCore::externalFM(int opNum, float extFM) {
    __opExtFM[opNum] = _mm_set1_ps(extFM);
}

void FourVoiceOPCore::externalSync(int opNum, float extSync) {
    if(extSync > 0 && !extSyncing) {
        extSyncing = true;
        __opExtSync[opNum] = __ones;
    }
    else {
        __opExtSync[opNum] = __zeros;
    }
    if(extSync <= 0) {
        extSyncing = false;
    }

}

void FourVoiceOPCore::setFrequency(int opNum, float frequency) {
    _op[opNum].setFrequency(frequency);
}

void FourVoiceOPCore::_mm_setFrequency(int opNum, const __m128& frequency) {
    _op[opNum].setFrequency(frequency);
}

void FourVoiceOPCore::setWavebank(int opNum, int bankNum) {
    float** wavebank;
    int32_t numWaves;
    int32_t tableSize;

    wavebank = wavetables[bankNum];
    numWaves = wavetable_sizes[bankNum];
    tableSize = wavetable_lengths[bankNum][0];
    _op[opNum].setWavebank(wavebank, numWaves, tableSize);
}

void FourVoiceOPCore::setWavePosition(int opNum, float position) {
    float scanPosition = position * ((float)_op[opNum].getNumwaves() - 1.f);
    _op[opNum].setScanPosition(scanPosition);
}

void FourVoiceOPCore::setShape(int opNum, float shape) {
    _op[opNum].setShape(shape);
}

void FourVoiceOPCore::setLevel(int opNum, float level) {
    _inLevels[opNum] = level;
}

void FourVoiceOPCore::setOpPreFade(int opNum, bool opPreFade) {
    _opPreFade[opNum] = opPreFade;
}

void FourVoiceOPCore::setBrightness(float brightness) {
    _brightness = brightness;
}

void FourVoiceOPCore::setSyncMode(int opNum, int syncMode) {
    _op[opNum].setSyncMode(syncMode);
}

void FourVoiceOPCore::setSyncSource(OpSyncSource opSyncSource) {
    _opSyncSource = opSyncSource;
}

void FourVoiceOPCore::enableSync(int opNum, bool enableSync) {
    _op[opNum].enableSync(enableSync);
}

void FourVoiceOPCore::enableIntSync(int opNum, bool enableSync) {
    if(enableSync) {
        _op[opNum].enableSync(enableSync);
        __opSyncEnable[opNum] = __ones;
    }
    else {
        __opSyncEnable[opNum] = __zeros;
    }
}

void FourVoiceOPCore::enableWeakSync(int opNum, bool weakEnable) {
    _op[opNum].enableWeakSync(weakEnable);
}

void FourVoiceOPCore::setShapeMode(int opNum, int shapeMode) {
    _op[opNum].setShapeMethod(shapeMode);
}

void FourVoiceOPCore::setPMPostShape(int opNum, bool PMPostShape) {
    _op[opNum].setPMPostShape(PMPostShape);
}

void FourVoiceOPCore::setSampleRate(float sampleRate) {
    _op[0].setSampleRate(sampleRate);
    _op[1].setSampleRate(sampleRate);
    _op[2].setSampleRate(sampleRate);
    _op[3].setSampleRate(sampleRate);
}

void FourVoiceOPCore::calcOpLevels() {
    int _opMask = 1;
    for(auto i = 0; i < kNumOperators; ++i) {
        _opLevels[i] = _inLevels[i];
        if(_opMask & _brightnessMask) {
            _opLevels[i] += _brightness;
        }
        if(_opLevels[i] < 0.f) {
            _opLevels[i] = 0.f;
        }
        else if(_opLevels[i] > 1.f) {
            _opLevels[i] = 1.f;
        }
        __opLevel[i] = _mm_set1_ps(_opLevels[i]);
        _opMask <<= 1;
    }
}

void FourVoiceOPCore::mix() {
    __opAuxOut[0] = _op[0].getOutput();
    __opAuxOut[1] = _op[1].getOutput();
    __opAuxOut[2] = _op[2].getOutput();
    __opAuxOut[3] = _op[3].getOutput();

    __opOut[0] = _mm_mul_ps(__opAuxOut[0], __opLevel[0]);
    __opOut[1] = _mm_mul_ps(__opAuxOut[1], __opLevel[1]);
    __opOut[2] = _mm_mul_ps(__opAuxOut[2], __opLevel[2]);
    __opOut[3] = _mm_mul_ps(__opAuxOut[3], __opLevel[3]);

    __op1Col = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][OP_0_COL]);
    __op2Col = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][OP_1_COL]);
    __op3Col = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][OP_2_COL]);
    __op4Col = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][OP_3_COL]);
    __mainCol = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][MAIN_OUT_COL]);
    __bCol = _mm_mul_ps(__opOut[0], __matrix[OP_0_ROW][B_OUT_COL]);

    __op1Col = _mm_add_ps(__op1Col, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][OP_0_COL]));
    __op2Col = _mm_add_ps(__op2Col, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][OP_1_COL]));
    __op3Col = _mm_add_ps(__op3Col, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][OP_2_COL]));
    __op4Col = _mm_add_ps(__op4Col, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][OP_3_COL]));
    __mainCol = _mm_add_ps(__mainCol, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][MAIN_OUT_COL]));
    __bCol = _mm_add_ps(__bCol, _mm_mul_ps(__opOut[1], __matrix[OP_1_ROW][B_OUT_COL]));

    __op1Col = _mm_add_ps(__op1Col, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][OP_0_COL]));
    __op2Col = _mm_add_ps(__op2Col, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][OP_1_COL]));
    __op3Col = _mm_add_ps(__op3Col, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][OP_2_COL]));
    __op4Col = _mm_add_ps(__op4Col, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][OP_3_COL]));
    __mainCol = _mm_add_ps(__mainCol, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][MAIN_OUT_COL]));
    __bCol = _mm_add_ps(__bCol, _mm_mul_ps(__opOut[2], __matrix[OP_2_ROW][B_OUT_COL]));

    __op1Col = _mm_add_ps(__op1Col, _mm_mul_ps(__opOut[3], __matrix[OP_3_ROW][OP_0_COL]));
    __op2Col = _mm_add_ps(__op2Col, _mm_mul_ps(__opOut[3], __matrix[OP_3_ROW][OP_1_COL]));
    __op3Col = _mm_add_ps(__op3Col, _mm_mul_ps(__opOut[3], __matrix[OP_3_ROW][OP_2_COL]));
    __op4Col = _mm_add_ps(__op4Col, _mm_mul_ps(_op[3].getOutput(), __matrix[OP_3_ROW][OP_3_COL])); // Allows feedback to OP3
    __mainCol = _mm_add_ps(__mainCol, _mm_mul_ps(__opOut[3], __matrix[OP_3_ROW][MAIN_OUT_COL]));   // independent of its level
    __bCol = _mm_add_ps(__bCol, _mm_mul_ps(__opOut[3], __matrix[OP_3_ROW][B_OUT_COL]));

    __op1Col = _mm_add_ps(__op1Col, __opExtFM[0]);
    __op2Col = _mm_add_ps(__op2Col, __opExtFM[1]);
    __op3Col = _mm_add_ps(__op3Col, __opExtFM[2]);
    __op4Col = _mm_add_ps(__op4Col, __opExtFM[3]);

    _op[0].setPhase(_mm_mul_ps(__op1Col, _mm_set1_ps(2.5f)));
    _op[1].setPhase(_mm_mul_ps(__op2Col, _mm_set1_ps(2.5f)));
    _op[2].setPhase(_mm_mul_ps(__op3Col, _mm_set1_ps(2.5f)));
    _op[3].setPhase(_mm_mul_ps(__op4Col, _mm_set1_ps(2.5f)));

    // Sync
    for(auto i = 0; i < kNumOperators; ++i) {
        __opSyncIn[i] = __zeros;
    }

    for(auto i = 0; i < kNumOperators; ++i) {
        __opSyncSignal[i] = _op[i].getEOCPulse();
        __opSyncIn[0] = _mm_add_ps(__opSyncIn[0], _mm_mul_ps(__opSyncSignal[i], __matrix[i][0]));
        __opSyncIn[1] = _mm_add_ps(__opSyncIn[1], _mm_mul_ps(__opSyncSignal[i], __matrix[i][1]));
        __opSyncIn[2] = _mm_add_ps(__opSyncIn[2], _mm_mul_ps(__opSyncSignal[i], __matrix[i][2]));
        __opSyncIn[3] = _mm_add_ps(__opSyncIn[3], _mm_mul_ps(__opSyncSignal[i], __matrix[i][3]));
    }

    if(_opSyncSource == NEIGHBOUR_SYNC_SOURCE) {
        __opSyncIn[0] = __opSyncSignal[1];
        __opSyncIn[1] = __opSyncSignal[2];
        __opSyncIn[2] = __opSyncSignal[3];
        __opSyncIn[3] = __zeros;
    }

    for(auto i = 0; i < kNumOperators; ++i) {
        __opSyncIn[i] = _mm_mul_ps(__opSyncIn[i], __opSyncEnable[i]);
        __opSyncIn[i] = _mm_add_ps(__opSyncIn[i], __opExtSync[i]);
        _op[i].sync(__opSyncIn[i]);
    }
}

void FourVoiceOPCore::clearMatrix() {
    for(auto i = 0; i < MatrixRows::NUM_ROWS; ++i) {
        for(auto j = 0; j < MatrixColumns::NUM_COLS; ++j) {
            __matrix[i][j] = __zeros;
        }
    }
}

void FourVoiceOPCore::setMatrixAlgo0() {
    //
    // [3]-->[2]-->[1]-->[0]--> A & B
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo1() {
    //
    // [3]-->+-->[1]-->[0]--> A & B
    // [2]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_1_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo2() {
    //
    // [2]-->[1]-->+-->[0]--> out
    //       [3]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_3_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo3() {
    //
    // [3]-->[2]-->+-->[0]--> out
    //       [1]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_0_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo4() {
    //
    //       |-->[2]-->|
    // [3]-->|         +-->[0]--> out
    //       |-->[1]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_0_COL] = __ones;
    __matrix[OP_3_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo5() {
    //
    //             |-->[1]-->|
    // [3]-->[2]-->|         +--> out
    //             |-->[0]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[1];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x0C;
}

void FourVoiceOPCore::setMatrixAlgo6() {
    //
    // [3]-->[2]-->[1]-->+--> outputs
    //             [0]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[1];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x0C;
}

void FourVoiceOPCore::setMatrixAlgo7() {
    //
    // [3]-->|
    // [2]-->+-->[0]--> out
    // [1]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_0_COL] = __ones;
    __matrix[OP_3_ROW][OP_0_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0E;
}

void FourVoiceOPCore::setMatrixAlgo8() {
    //
    // [3]-->[2]-->|
    //             +--> out
    // [1]-->[0]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[1];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x0A;
}

void FourVoiceOPCore::setMatrixAlgo9() {
    //
    //       |-->[2]-->|
    // [3]-->|-->[1]-->+--> out
    //       |-->[0]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_0_COL] = __ones;
    __matrix[OP_3_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[2];
    __bOutLevel = __outputLevels[2];
    _brightnessMask = 0x08;
}

void FourVoiceOPCore::setMatrixAlgo10() {
    //
    // [3]-->[2]-->|
    //       [1]-->+--> out
    //       [0]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[2];
    __bOutLevel = __outputLevels[2];
    _brightnessMask = 0x08;
}

void FourVoiceOPCore::setMatrixAlgo11() {
    // All going to all outputs
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_3_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[3];
    __bOutLevel = __outputLevels[3];
    _brightnessMask = 0x00;
}

void FourVoiceOPCore::setMatrixAlgo12() {
    //
    // [1]-->[0]--> A
    //
    // [3]-->[2]--> B
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0A;
}

void FourVoiceOPCore::setMatrixAlgo13() {
    //
    // [2]-->[1]-->[0]--> A
    //
    // [3]--> B
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x06;
}

void FourVoiceOPCore::setMatrixAlgo14() {
    //
    // [3]-->[2]-->[1]--> A
    //
    // [0]--> B
    //
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0C;
}

void FourVoiceOPCore::setMatrixAlgo15() {
    //
    // [3]--> A
    //
    // [2]-->[1]-->[0]--> B
    //
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][MAIN_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0C;
}

void FourVoiceOPCore::setMatrixAlgo16() {
    //
    // [0]--> A
    //
    // [3]-->[2]-->[1]--> B
    //
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][OP_1_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x0C;
}

void FourVoiceOPCore::setMatrixAlgo17() {
    //
    // [1]-->[0]--> A
    //
    // [2]-->+--> B
    // [3]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][OP_0_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x02;
}

void FourVoiceOPCore::setMatrixAlgo18() {
    //
    // [3]-->[2]--> A
    //
    // [0]-->+--> B
    // [1]-->|
    //
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x08;
}

void FourVoiceOPCore::setMatrixAlgo19() {
    //
    // [0]-->+--> A
    // [1]-->|
    //
    // [3]-->[2]--> B
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][OP_2_COL] = __ones;
    __aOutLevel = __outputLevels[1];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x08;
}

void FourVoiceOPCore::setMatrixAlgo20() {
    //
    // [0]-->+--> A
    // [1]-->|
    //
    // [2]-->+--> B
    // [3]-->|
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[1];
    __bOutLevel = __outputLevels[1];
    _brightnessMask = 0x00;
}

void FourVoiceOPCore::setMatrixAlgo21() {
    //
    // [0]-->|
    // [1]-->+--> A
    // [2]-->|
    //
    // [3]--> B
    //
    __matrix[OP_0_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_1_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_2_ROW][MAIN_OUT_COL] = __ones;
    __matrix[OP_3_ROW][B_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[2];
    __bOutLevel = __outputLevels[0];
    _brightnessMask = 0x00;
}

void FourVoiceOPCore::setMatrixAlgo22() {
    //
    // [3]-->+--> A
    //
    // [0]-->|
    // [1]-->+--> B
    // [2]-->|
    //
    __matrix[OP_0_ROW][B_OUT_COL] = __ones;
    __matrix[OP_1_ROW][B_OUT_COL] = __ones;
    __matrix[OP_2_ROW][B_OUT_COL] = __ones;
    __matrix[OP_3_ROW][MAIN_OUT_COL] = __ones;
    __aOutLevel = __outputLevels[0];
    __bOutLevel = __outputLevels[2];
    _brightnessMask = 0x00;
}
