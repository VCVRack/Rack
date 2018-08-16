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

#ifndef PEAKS_DRIVERS_DAC_H_
#define PEAKS_DRIVERS_DAC_H_

#include <stm32f10x_conf.h>

#include "stmlib/stmlib.h"

namespace peaks {

const uint16_t kPinSS = GPIO_Pin_12;

class Dac {
 public:
  Dac() { }
  ~Dac() { }

  void Init();

  inline void Write(int index, uint16_t value) {
    data_[index] = value;
  }

  inline bool Update() {
    GPIOB->BSRR = kPinSS;
    GPIOB->BRR = kPinSS;
    
    if (wrote_both_channels_) {
      SPI2->DR = 0x2400 | (data_[0] >> 8);
      wrote_both_channels_ = false;
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      SPI2->DR = data_[0] << 8;
    } else {
      SPI2->DR = 0x1000 | (data_[1] >> 8);
      wrote_both_channels_ = true;
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      SPI2->DR = data_[1] << 8;
    }
    return wrote_both_channels_;
  }
  
  void Write(uint16_t channel_1);

 private:
  uint16_t data_[2];
  bool wrote_both_channels_;
  
  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace peaks

#endif  // PEAKS_DRIVERS_DAC_H_
