// Copyright 2017 Olivier Gillet.
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
// Driver for the six gate/trigger inputs.

#ifndef STAGES_DRIVERS_GATE_INPUTS_H_
#define STAGES_DRIVERS_GATE_INPUTS_H_

#include "stmlib/stmlib.h"

#include "stages/io_buffer.h"

namespace stages {
  
class GateInputs {
 public:
  GateInputs() { }
  ~GateInputs() { }
  
  void Init();
  void Read(const IOBuffer::Slice& slice, size_t size);
  void ReadNormalization(IOBuffer::Block* block);
  
  inline bool is_normalized(int i) const {
    return normalized_[i];
  }
  
  inline bool value(int i) const {
    return previous_flags_[i] & stmlib::GATE_FLAG_HIGH;
  }
  
 private:
  static const int kProbeSequenceDuration = 64;

  stmlib::GateFlags previous_flags_[kNumChannels];

  uint32_t normalization_probe_state_;
  bool normalized_[kNumChannels];
  int normalization_mismatch_count_[kNumChannels];
  int normalization_decision_count_;
  
  DISALLOW_COPY_AND_ASSIGN(GateInputs);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_GATE_INPUTS_H_
