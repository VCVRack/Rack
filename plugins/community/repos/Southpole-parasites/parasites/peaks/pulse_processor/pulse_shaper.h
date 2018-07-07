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
// Trigger to gate converter with pre-delay, duration, and repetitions.

#ifndef PEAKS_PULSE_PROCESSOR_PULSE_SHAPER_H_
#define PEAKS_PULSE_PROCESSOR_PULSE_SHAPER_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

#include "peaks/gate_processor.h"

namespace peaks {

struct Pulse {
  uint16_t initial_delay_counter;
  uint16_t duration_counter;
  uint16_t delay_counter;
  uint16_t repetition_counter;
};

static const uint8_t kPulseBufferSize = 32;

class PulseShaper {
 public:
  PulseShaper() { }
  ~PulseShaper() { }
  
  void Init();
  void FillBuffer(InputBuffer* input_buffer, OutputBuffer* output_buffer);
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_initial_delay(0);
      set_duration(parameter[0] >> 1);
      set_delay((parameter[0] >> 1) + 2048);
      set_num_repetitions(parameter[1]);
    } else {
      set_initial_delay(parameter[0]);
      set_duration(parameter[1] >> 1);
      set_delay(parameter[2] >> 1);
      set_num_repetitions(parameter[3]);
    }
  }

  inline void set_initial_delay(uint16_t initial_delay) {
    initial_delay_ = initial_delay;
  }
  
  inline void set_duration(uint16_t duration) {
    duration_ = duration;
  }
  
  inline void set_delay(uint16_t delay) {
    delay_ = delay;
  }
  
  inline void set_num_repetitions(uint16_t num_repetitions) {
    num_repetitions_ = num_repetitions >> 13;
  }
  
 private:
  uint16_t delay() const;
  uint16_t duration() const;
  uint16_t initial_delay() const;
  
  uint16_t initial_delay_;
  uint16_t duration_;
  uint16_t delay_;
  uint16_t num_repetitions_;
  
  uint16_t previous_num_pulses_;
  uint16_t retrig_counter_;
  
  Pulse pulse_buffer_[kPulseBufferSize];

  DISALLOW_COPY_AND_ASSIGN(PulseShaper);
};

}  // namespace peaks

#endif  // PEAKS_PULSE_PROCESSOR_PULSE_SHAPER_H_
