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

#ifndef TIDES_DRIVERS_DAC_H_
#define TIDES_DRIVERS_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f10x_conf.h>

namespace tides {

const uint16_t kPinSS = GPIO_Pin_12;

class Dac {
 public:
  Dac() { }
  ~Dac() { }
  
  void Init();
  
  void Write(uint16_t channel_1, uint16_t channel_2) {
    data_[0] = channel_1;
    data_[1] = channel_2;
  }
  
  bool ready() { return active_channel_ == 0; }
  
  inline void Update() {
    GPIOB->BSRR = kPinSS;
    GPIOB->BRR = kPinSS;
    if (active_channel_ == 0) {
      SPI2->DR = 0x1000 | (data_[0] >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (data_[0] << 8) & 0xffff;
      active_channel_ = 1;
    } else {
      SPI2->DR = 0x2400 | (data_[1] >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (data_[1] << 8) & 0xffff;
      active_channel_ = 0;
    }
  }
  
  inline void Update(uint8_t channel) {
    GPIOB->BSRR = kPinSS;
    GPIOB->BRR = kPinSS;
    if (channel == 0) {
      SPI2->DR = 0x1000 | (data_[0] >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (data_[0] << 8) & 0xffff;
    } else {
      SPI2->DR = 0x2400 | (data_[1] >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (data_[1] << 8) & 0xffff;
    }
  }
 
 private:
  uint16_t data_[2];
  uint8_t active_channel_;
  
  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace tides

#endif  // TIDES_DRIVERS_DAC_H_
