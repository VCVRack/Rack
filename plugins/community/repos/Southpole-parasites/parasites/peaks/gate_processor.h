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

enum ControlBitMask {
  CONTROL_GATE = 1,
  CONTROL_GATE_RISING = 2,
  CONTROL_GATE_FALLING = 4,

  CONTROL_GATE_AUXILIARY = 16,
  CONTROL_GATE_RISING_AUXILIARY = 32,
  CONTROL_GATE_FALLING_AUXILIARY = 64
};

enum ControlMode {
  CONTROL_MODE_FULL,
  CONTROL_MODE_HALF
};

const uint16_t kBlockSize = 8;

typedef stmlib::RingBuffer<uint8_t, kBlockSize * 2> InputBuffer;
typedef stmlib::RingBuffer<int16_t, kBlockSize * 2> OutputBuffer;

}  // namespace peaks

#endif  // PEAKS_GATE_PROCESSOR_H_
