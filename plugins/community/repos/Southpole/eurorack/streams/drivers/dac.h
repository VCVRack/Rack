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
// Driver for DAC (controlling VCA).

#ifndef STREAMS_DRIVERS_DAC_H_
#define STREAMS_DRIVERS_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f10x_conf.h>

namespace streams {

const uint16_t kPinSS = GPIO_Pin_12;

class Dac {
 public:
  Dac() { }
  ~Dac() { }
  
  void Init();
  
  void Write(uint8_t channel, uint16_t value) {
    GPIOB->BSRR = kPinSS;
    GPIOB->BRR = kPinSS;
    if (channel == 1) {
      SPI2->DR = 0x1000 | (value >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (value << 8) & 0xffff;
    } else {
      SPI2->DR = 0x2400 | (value >> 8);
      while (!(SPI2->SR & SPI_I2S_FLAG_RXNE));
      SPI2->DR = (value << 8) & 0xffff;
    }
  }
 
 private:
  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace streams

#endif  // STREAMS_DRIVERS_DAC_H_
