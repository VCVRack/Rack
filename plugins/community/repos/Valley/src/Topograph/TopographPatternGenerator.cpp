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

#include "TopographPatternGenerator.hpp"

uint8_t U8U8MulShift8(uint8_t a, uint8_t b) {
    return (a * b) >> 8;
}

uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance) {
    return (a * (255 - balance) + b * balance) / 255;
}

PatternGenerator::PatternGenerator() {
    _pulse = 0;
    _beat = 0;
    _firstBeat = 0;
    _step = 0;
    for(int i = 0; i < 3; ++i) {
        _euclideanStep[i] = 0;
    }
    _state = 0;
    _accentBits = 0;
}

void PatternGenerator::tick(uint8_t numPulses) {
    evaluate();
    _beat = (_step & 0x7) == 0;
    _firstBeat = _step == 0;
    _pulse += numPulses;

    // Wrap into ppqn steps.
    while (_pulse >= kPulsesPerStep) {
        _pulse -= kPulsesPerStep;
        if (!(_step & 1)) {
            for (uint8_t i = 0; i < kNumParts; ++i) {
                ++_euclideanStep[i];
            }
        }
        ++_step;
    }

    // Wrap into step sequence steps.
    if (_step >= kStepsPerPattern) {
        _step -= kStepsPerPattern;
    }
}

void PatternGenerator::reset() {
    _step = 0;
    _pulse = 0;
    for(long i = 0; i < 3; ++i) {
        _euclideanStep[i] = 0;
    }
}

void PatternGenerator::setMapX(uint8_t x) {
    _settings.x = x;
}

void PatternGenerator::setMapY(uint8_t y) {
    _settings.y = y;
}

void PatternGenerator::setBDDensity(uint8_t density) {
    _settings.density[0] = density;
}

void PatternGenerator::setSDDensity(uint8_t density) {
    _settings.density[1] = density;
}

void PatternGenerator::setHHDensity(uint8_t density) {
    _settings.density[2] = density;
}

void PatternGenerator::setDrumDensity(uint8_t channel, uint8_t density) {
    _settings.density[channel] = density;
}

void PatternGenerator::setEuclideanLength(uint8_t channel, uint8_t length){
    _settings.euclidean_length[channel] = length;
}

void PatternGenerator::setRandomness(uint8_t randomness) {
    _settings.randomness = randomness;
}

void PatternGenerator::setAccentAltMode(bool accAlt){
    _settings.accAlt = accAlt;
}

void PatternGenerator::setPatternMode(PatternGeneratorMode mode) {
    _settings.patternMode = mode;
}

uint8_t PatternGenerator::getAllStates() const {
    return _state;
}

uint8_t PatternGenerator::getDrumState(uint8_t channel) const {
    uint8_t mask[6] = {1,2,4,8,16,32};
    return (_state & mask[channel]) >> channel;
}

PatternGeneratorMode PatternGenerator::getPatternMode() const {
    return _settings.patternMode;
}

uint8_t PatternGenerator::getBeat() const {
    return _beat;
}

uint8_t PatternGenerator::getEuclideanLength(uint8_t channel) {
    return _settings.euclidean_length[channel];
}

uint8_t PatternGenerator::readDrumMap(uint8_t step, uint8_t instrument, uint8_t x, uint8_t y) {
    uint8_t r = 0;
    if(_settings.patternMode == PATTERN_HENRI) {
        uint8_t i = (int)floor(x * 3.0 / 255.0);
        uint8_t j = (int)floor(y * 3.0 / 255.0);
        const uint8_t* a_map = drum_map[i][j];
        const uint8_t* b_map = drum_map[i + 1][j];
        const uint8_t* c_map = drum_map[i][j + 1];
        const uint8_t* d_map = drum_map[i + 1][j + 1];
        uint8_t offset = (instrument * kStepsPerPattern) + step;
        uint8_t a = a_map[offset];
        uint8_t b = b_map[offset];
        uint8_t c = c_map[offset];
        uint8_t d = d_map[offset];
        uint8_t maxValue = 127;
        r = (( a * x + b * (maxValue - x) ) * y + (c * x + d * (maxValue - x)) *
             ( maxValue - y )) / maxValue / maxValue;
    }
    else {
        uint8_t i = x >> 6;
        uint8_t j = y >> 6;
        const uint8_t* a_map = drum_map[i][j];
        const uint8_t* b_map = drum_map[i + 1][j];
        const uint8_t* c_map = drum_map[i][j + 1];
        const uint8_t* d_map = drum_map[i + 1][j + 1];
        uint8_t offset = (instrument * kStepsPerPattern) + step;
        uint8_t a = *(a_map + offset);
        uint8_t b = *(b_map + offset);
        uint8_t c = *(c_map + offset);
        uint8_t d = *(d_map + offset);
        r = U8Mix(U8Mix(a, b, x << 2), U8Mix(c, d, x << 2), y << 2);
    }

    return r;
}

void PatternGenerator::evaluate() {
    _state = 0;
    _state |= 0x40;

    if (_settings.accAlt) {
        _state |= OUTPUT_BIT_CLOCK;
    }

    // Refresh only at step changes.
    if (_pulse != 0) {
        return;
    }

    if (_settings.patternMode == PATTERN_EUCLIDEAN) {
        evaluateEuclidean();
    } else {
        evaluateDrums();
    }
}

void PatternGenerator::evaluateEuclidean() {
    // Refresh only on sixteenth notes.
    if (_step & 1) {
        return;
    }

    // Euclidean pattern generation
    uint8_t instrument_mask = 1;
    uint8_t reset_bits = 0;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        uint8_t length = (_settings.euclidean_length[i] >> 3) + 1;
        uint8_t density = _settings.density[i] >> 3;
        uint16_t address = (length - 1) * 32 + density;
        while (_euclideanStep[i] >= length) {
            _euclideanStep[i] -= length;
        }
        uint32_t step_mask = 1L << static_cast<uint32_t>(_euclideanStep[i]);
        uint32_t pattern_bits = *(lut_res_euclidean + address);
        if (pattern_bits & step_mask) {
            _state |= instrument_mask;
        }
        if (_euclideanStep[i] == 0) {
            reset_bits |= instrument_mask;
        }
        instrument_mask <<= 1;
    }

    if (_settings.accAlt) {
        _state |= reset_bits ? OUTPUT_BIT_COMMON : 0;
        _state |= (reset_bits == 0x07) ? OUTPUT_BIT_RESET : 0;
    } else {
        _state |= reset_bits << 3;
    }
}

void PatternGenerator::evaluateDrums() {
    // At the beginning of a pattern, decide on perturbation levels.
    if (_step == 0) {
        for (uint8_t i = 0; i < kNumParts; ++i) {
            uint8_t randomNum = (uint8_t)rand() % 256;
            uint8_t randomness = _settings.swing ? 0 : _settings.randomness >> 2;
            _partPerturbation_[i] = U8U8MulShift8(randomNum, randomness);
        }
    }

    uint8_t instrument_mask = 1;
    uint8_t x = _settings.x;
    uint8_t y = _settings.y;
    _accentBits = 0;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        uint8_t level = readDrumMap(_step, i, x, y);
        if (level < 255 - _partPerturbation_[i]) {
            level += _partPerturbation_[i];
        }
        else {
            // The sequencer from Anushri uses a weird clipping rule here. Comment
            // this line to reproduce its behavior.
            level = 255;
        }
        uint8_t threshold = ~_settings.density[i];
        if (level > threshold) {
            if (level > 192) {
                _accentBits |= instrument_mask;
            }
            _state |= instrument_mask;
        }
        instrument_mask <<= 1;
    }
    if (_settings.accAlt) {
        _state |= _accentBits ? OUTPUT_BIT_COMMON : 0;
        _state |= _step == 0 ? OUTPUT_BIT_RESET : 0;
    }
    else {
        _state |= _accentBits << 3;
    }

}
