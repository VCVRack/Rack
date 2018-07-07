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
// Driver for PWM (controlling VCF).

#ifndef STREAMS_DRIVERS_PWM_H_
#define STREAMS_DRIVERS_PWM_H_

#include "stmlib/stmlib.h"

#include <stm32f10x_conf.h>

namespace streams {

const uint16_t kPwmResolution = 10;  // 10 bits -> 70kHz PWM.

class Pwm {
 public:
  Pwm() { }
  ~Pwm() { }
  
  void Init();
  
  void Write(uint8_t channel, uint16_t value) {
    filtered_value_[channel] += (value - filtered_value_[channel]) >> 4;
    uint16_t channel_value = filtered_value_[channel] >> (16 - kPwmResolution);
    if (channel == 0) {
      TIM1->CCR3 = channel_value;
    } else {
      TIM1->CCR4 = channel_value;
    }
  }
  
 private:
  uint16_t filtered_value_[2];
  DISALLOW_COPY_AND_ASSIGN(Pwm);
};

}  // namespace streams

#endif  // STREAMS_DRIVERS_DAC_H_
