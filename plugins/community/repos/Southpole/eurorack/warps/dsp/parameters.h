// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Parameters of the modulator.

#ifndef WARPS_DSP_PARAMETERS_H_
#define WARPS_DSP_PARAMETERS_H_

#include "stmlib/stmlib.h"

namespace warps {
  
enum OscillatorShape {
  OSCILLATOR_SHAPE_SINE,
  OSCILLATOR_SHAPE_TRIANGLE,
  OSCILLATOR_SHAPE_SAW,
  OSCILLATOR_SHAPE_PULSE,
  OSCILLATOR_SHAPE_NOISE_LP
};

enum ModulationAlgorithm {
  MODULATION_ALGORITHM_XFADE,
  MODULATION_ALGORITHM_FOLD,
  MODULATION_ALGORITHM_ANALOG_RINGMOD,
  MODULATION_ALGORITHM_DIGITAL_RINGMOD,
  MODULATION_ALGORITHM_XOR,
  MODULATION_ALGORITHM_COMPARE,
  MODULATION_ALGORITHM_SPECTRAL,
  MODULATION_ALGORITHM_MORPH,
  MODULATION_ALGORITHM_VOCODER
};

struct Parameters {
  float channel_drive[2];
  float modulation_algorithm;
  float modulation_parameter;
  
  // Easter egg parameters.
  float frequency_shift_pot;
  float frequency_shift_cv;
  float phase_shift;
  float note;

  int32_t carrier_shape;  // 0 = external
  
  // Apply a non-linear response to the parameter of all algorithms between
  // 1 and 4.
  inline float skewed_modulation_parameter() const {
    float skew = 0.0f;
    if (modulation_algorithm <= 1.0f) {
      skew = modulation_algorithm;
    } else if (modulation_algorithm >= 5.0f) {
      skew = 1.0f;
    } else if (modulation_algorithm >= 4.0f) {
      skew = 5.0f - modulation_algorithm;
    }
    return modulation_parameter * (1.0f + skew * (modulation_parameter - 1.0f));
  }
};

}  // namespace warps

#endif  // WARPS_DSP_PARAMETERS_H_
