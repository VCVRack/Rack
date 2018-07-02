// Copyright 2013 Olivier Gillet.
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
// General declarations used by all trigger/gate processors.

#ifndef PEAKS_GATE_PROCESSOR_H_
#define PEAKS_GATE_PROCESSOR_H_

#include "stmlib/stmlib.h"

#include "stmlib/utils/ring_buffer.h"

namespace peaks {

enum ControlMode {
  CONTROL_MODE_FULL,
  CONTROL_MODE_HALF
};

enum GateFlagsBits {
  GATE_FLAG_LOW = 0,
  GATE_FLAG_HIGH = 1,
  GATE_FLAG_RISING = 2,
  GATE_FLAG_FALLING = 4,
  GATE_FLAG_FROM_BUTTON = 8,
  
  GATE_FLAG_AUXILIARY_LOW = 0,
  GATE_FLAG_AUXILIARY_HIGH = 16,
  GATE_FLAG_AUXILIARY_RISING = 32,
  GATE_FLAG_AUXILIARY_FALLING = 64,
};

typedef uint8_t GateFlags;

inline GateFlags ExtractGateFlags(GateFlags previous, bool current) {
  previous &= GATE_FLAG_HIGH;
  if (current) {
    return previous ? GATE_FLAG_HIGH : (GATE_FLAG_RISING | GATE_FLAG_HIGH);
  } else {
    return previous ? GATE_FLAG_FALLING : GATE_FLAG_LOW;
  }
}

}  // namespace peaks

#endif  // PEAKS_GATE_PROCESSOR_H_
