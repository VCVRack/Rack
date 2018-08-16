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
// Value to output on the DAC to get unitary gain.

#ifndef STREAMS_GAIN_H_
#define STREAMS_GAIN_H_

namespace streams {

// The DAC sends up to 2.5V through 25k = 0.1mA over the range of the DAC.
// The offness compensation resistor sends -10V through 10M = 0.001mA.
// Assuming the op-amp offset is negligible, the two balance for a DAC code of
// 65536 / 100.0
  
const int32_t kDefaultOffset = 655;

// DAC code giving a unitary gain.
const int32_t kUnityGain = 32767;

// Slightly above unitary gain.
const int32_t kAboveUnityGain = 32896;

// Maximum gain in dB in lin mode with a DAC code of 65535 (6 dB).
const int32_t kMaxLinearGain = 65536;
// Maximum gain in dB in lin mode with a DAC code of 65535 (18 dB).
const int32_t kMaxExponentialGain = 218453;

const uint16_t kSchmittTriggerThreshold = 32768 * 5 * 2 / 3 / 8;

}  // namespace streams

#endif  // STREAMS_GAIN_H_