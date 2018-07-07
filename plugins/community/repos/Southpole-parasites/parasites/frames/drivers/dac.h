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
// Driver for DAC.

#ifndef FRAMES_DRIVERS_DAC_H_
#define FRAMES_DRIVERS_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f10x_conf.h>

#include "frames/keyframer.h"

namespace frames {

const uint16_t kPinSS = GPIO_Pin_12;

class Dac {
 public:
  Dac() { }
  ~Dac() { }
  
  void Init();
  void Write(uint8_t channel, uint16_t data) {
    data_[channel] = data & 0x0fff;
  }
  void Update() {
    GPIOB->BSRR = kPinSS;
    GPIOB->BRR = kPinSS;
    uint16_t command_nibble = (active_channel_ << 2) | 1;
    SPI2->DR = (command_nibble << 12) | data_[active_channel_];
    active_channel_ = (active_channel_ + 1) % 4;
  }
  bool ready() { return active_channel_ == 0; }
 
 private:
  uint8_t active_channel_;
  uint16_t data_[kNumChannels];
  
  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace frames

#endif  // FRAMES_DRIVERS_DAC_H_
