// Copyright 2012 Olivier Gillet.
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
// Driver for ADC scanning.

#ifndef BRAIDS_DRIVERS_ADC_H_
#define BRAIDS_DRIVERS_ADC_H_

#include <stm32f10x_conf.h>
#include "stmlib/stmlib.h"

namespace braids {

const size_t kNumChannels = 4;

class Adc {
 public:
  Adc() { }
  ~Adc() { }
  
  void Init(bool double_speed);
  
  // Inlined and optimized!
  inline bool PipelinedScan() {
    switch (acquisition_stage_) {
      case 0:
        rx_word_ |= SPI1->DR;
        channels_[active_channel_] = rx_word_;
        GPIOB->BSRR = GPIO_Pin_0;
        GPIOB->BRR = GPIO_Pin_0;
        SPI1->DR = 0x04 | 0x02;
        active_channel_ = (active_channel_ + 1) % kNumChannels;
        acquisition_stage_ = 1;
        break;

      case 1:
        SPI1->DR;
        SPI1->DR = active_channel_ << 6;
        acquisition_stage_ = 2;
        break;

      case 2:
        rx_word_ = (SPI1->DR & 0xf) << 8;
        SPI1->DR = 0x00;  // Dummy trailing data.
        acquisition_stage_ = 0;
        break;
    }
    return (active_channel_ == 0 && acquisition_stage_ == 1);
  }

  bool PipelinedRead(uint8_t channel);

  uint16_t Read(uint8_t channel);
  uint16_t channel(uint8_t index) const { return channels_[index]; }
 
 private:
  uint16_t rx_word_;
  size_t active_channel_;
  size_t acquisition_stage_;
  uint16_t channels_[kNumChannels];

  DISALLOW_COPY_AND_ASSIGN(Adc);
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_ADC_H_
