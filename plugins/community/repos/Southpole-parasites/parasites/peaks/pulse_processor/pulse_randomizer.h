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
// Create random repetitions of a pulse.

#ifndef PEAKS_PULSE_PROCESSOR_PULSE_RANDOMIZER_H_
#define PEAKS_PULSE_PROCESSOR_PULSE_RANDOMIZER_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

#include "peaks/gate_processor.h"

namespace peaks {

struct TriggerPulse {
  uint32_t delay_counter;
};

static const uint8_t kTriggerPulseBufferSize = 32;

class PulseRandomizer {
 public:
  PulseRandomizer() { }
  ~PulseRandomizer() { }
  
  void Init();
  void FillBuffer(InputBuffer* input_buffer, OutputBuffer* output_buffer);
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_acceptance_probability(65535);
      set_repetition_probability(parameter[0]);
      set_delay_average(parameter[1]);
      set_delay_randomness(0);
    } else {
      set_acceptance_probability(parameter[0]);
      set_repetition_probability(parameter[1]);
      set_delay_average(parameter[2]);
      set_delay_randomness(parameter[3]);
    }
  }

  inline void set_repetition_probability(uint16_t repetition_probability) {
    repetition_probability_ = repetition_probability;
  }
  
  inline void set_acceptance_probability(uint16_t acceptance_probability) {
    acceptance_probability_ = acceptance_probability;
  }
  
  inline void set_delay_average(uint16_t delay_average) {
    delay_average_ = delay_average >> 1;
  }
  
  inline void set_delay_randomness(uint16_t delay_randomness) {
    delay_randomness_ = delay_randomness;
  }
  
 private:
  uint16_t delay() const;
  
  uint16_t repetition_probability_;
  uint16_t acceptance_probability_;
  uint16_t delay_average_;
  uint16_t delay_randomness_;

  uint16_t num_pulses_;
  uint16_t retrig_counter_;
  
  uint16_t delay_counter_[kTriggerPulseBufferSize];

  DISALLOW_COPY_AND_ASSIGN(PulseRandomizer);
};

}  // namespace peaks

#endif  // PEAKS_PULSE_PROCESSOR_PULSE_RANDOMIZER_H_
