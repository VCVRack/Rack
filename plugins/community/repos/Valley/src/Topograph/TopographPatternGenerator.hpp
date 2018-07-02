//
// Topograph
// A port of "Mutable Instruments Grids" for VCV Rack
// Author: Dale Johnson (valley.audio.soft@gmail.com)
// Date: 4/12/2017
//
// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Pattern generator.
//
// OUTPUT MODE  OUTPUT CLOCK  BIT7  BIT6  BIT5  BIT4  BIT3  BIT2  BIT1  BIT0
// DRUMS        FALSE          RND   CLK  HHAC  SDAC  BDAC    HH    SD    BD
// DRUMS        TRUE           RND   CLK   CLK   BAR   ACC    HH    SD    BD
// EUCLIDEAN    FALSE          RND   CLK  RST3  RST2  RST1  EUC3  EUC2  EUC1
// EUCLIDEAN    TRUE           RND   CLK   CLK  STEP   RST  EUC3  EUC2  EUC1

#ifndef TopographPatternGenerator_hpp
#define TopographPatternGenerator_hpp

#include <cstdlib>
#include <cmath>
#include "TopographResources.hpp"

const uint8_t kNumParts = 3;
const uint8_t kPulsesPerStep = 3;  // 24 ppqn ; 8 steps per quarter note.
const uint8_t kStepsPerPattern = 32;
const uint8_t kPulseDuration = 8;  // 8 ticks of the main clock.

uint8_t U8U8MulShift8(uint8_t a, uint8_t b);
uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance);

enum OutputBits {
    OUTPUT_BIT_COMMON = 0x08,
    OUTPUT_BIT_CLOCK = 0x10,
    OUTPUT_BIT_RESET = 0x20
};

static const uint8_t* drum_map[5][5] = {
    { node_10, node_8, node_0, node_9, node_11 },
    { node_15, node_7, node_13, node_12, node_6 },
    { node_18, node_14, node_4, node_5, node_3 },
    { node_23, node_16, node_21, node_1, node_2 },
    { node_24, node_19, node_17, node_20, node_22 },
};

enum PatternGeneratorMode {
    PATTERN_HENRI,
    PATTERN_OLIVIER,
    PATTERN_EUCLIDEAN
};

enum ClockResolution {
    CLOCK_RESOLUTION_4_PPQN,
    CLOCK_RESOLUTION_8_PPQN,
    CLOCK_RESOLUTION_24_PPQN,
};

const uint8_t ticks_granularity[] = { 6, 3, 1 };

struct PatternGeneratorOptions {
    PatternGeneratorOptions() {
        x = 0;
        y = 0;
        randomness = 0;
        for(int i = 0; i < kNumParts; ++i) {
            euclidean_length[i] = 255;
            density[i] = 0;
        }
        patternMode = PATTERN_HENRI;
        swing = false;
        accAlt = false;
    }
    uint8_t x;
    uint8_t y;
    uint8_t randomness;
    uint8_t euclidean_length[kNumParts];
    uint8_t density[kNumParts];
    PatternGeneratorMode patternMode;
    bool accAlt;
    bool swing;
};

class PatternGenerator {
public:
    PatternGenerator();
    void tick(uint8_t numPulses);
    void reset();

    void setMapX(uint8_t x);
    void setMapY(uint8_t y);
    void setBDDensity(uint8_t density);
    void setSDDensity(uint8_t density);
    void setHHDensity(uint8_t density);
    void setDrumDensity(uint8_t channel, uint8_t density);
    void setEuclideanLength(uint8_t channel, uint8_t length);
    void setRandomness(uint8_t randomness);
    void setAccentAltMode(bool accAlt);
    void setPatternMode(PatternGeneratorMode mode);

    uint8_t getAllStates() const;
    uint8_t getDrumState(uint8_t channel) const;
    PatternGeneratorMode getPatternMode() const;
    uint8_t getBeat() const;
    uint8_t getEuclideanLength(uint8_t channel);
private:
    PatternGeneratorOptions _settings;
    uint8_t _pulse;
    uint8_t _beat;
    uint8_t _firstBeat;
    uint8_t _step;
    uint8_t _euclideanStep[3];
    uint8_t _state;
    uint8_t _accentBits;

    uint8_t _partPerturbation_[kNumParts];
    uint8_t readDrumMap(uint8_t step, uint8_t instrument, uint8_t x, uint8_t y);
    void evaluate();
    void evaluateEuclidean();
    void evaluateDrums();
};

#endif /* TopographPatternGenerator_hpp */
