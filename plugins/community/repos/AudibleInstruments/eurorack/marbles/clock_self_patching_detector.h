// Copyright 2015 Olivier Gillet.
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
// Class for detecting if the t1 or t2 gate outputs are patched into the X
// clock input. This is done by comparing the number of synchronous transitions
// on the gate ouputs and the clock input. A small margin of error is allowed
// because of acquisition delays.

#ifndef MARBLES_CLOCK_SELF_PATCHING_DETECTOR_H_
#define MARBLES_CLOCK_SELF_PATCHING_DETECTOR_H_

#include "stmlib/stmlib.h"

#include "marbles/io_buffer.h"

namespace marbles {

class ClockSelfPatchingDetector {
 public:
  ClockSelfPatchingDetector() { }
  ~ClockSelfPatchingDetector() { }
  
  void Init(size_t index) {
    index_ = index;
    error_streak_ = 0;
    match_length_ = 0;
    synchronous_transitions_ = 0;
  }
  
  size_t Process(IOBuffer::Block* block, size_t size) {
    for (size_t i = 0; i < size; ++i) {
      if (block->input[1][i] & stmlib::GATE_FLAG_RISING) {
        if (match_length_ >= 12) {
          ++synchronous_transitions_;
        }
        error_streak_ = 0;
        match_length_ = 0;
      }
      bool output_gate = block->gate_output[index_][i];
      bool input_gate = block->input[1][i] & stmlib::GATE_FLAG_HIGH;
      if (output_gate != input_gate) {
        ++error_streak_;
        if (error_streak_ >= 6) {
          synchronous_transitions_ = 0;
        }
      } else {
        ++match_length_;
      }
    }
    return synchronous_transitions_;
  }
  
 private:
  size_t index_;
  size_t error_streak_;
  size_t match_length_;
  size_t synchronous_transitions_;
  
  DISALLOW_COPY_AND_ASSIGN(ClockSelfPatchingDetector);
};

}  // namespace marbles

#endif  // MARBLES_CLOCK_SELF_PATCHING_DETECTOR_H_
