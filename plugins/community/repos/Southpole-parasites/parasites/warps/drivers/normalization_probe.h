// Copyright 2014 Olivier Gillet.
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
// Driver for the normalization probe.

#ifndef WARPS_DRIVERS_NORMALIZATION_PROBE_H_
#define WARPS_DRIVERS_NORMALIZATION_PROBE_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

namespace warps {

class NormalizationProbe {
 public:
  NormalizationProbe() { }
  ~NormalizationProbe() { }
  
  void Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_8;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &gpio_init);
  }

  static inline void High() {
    GPIOC->BSRRL = GPIO_Pin_8;
  }
  
  static inline void Low() {
    GPIOC->BSRRH = GPIO_Pin_8;
  }
  
  static inline void Write(bool value) {
    if (value) {
      High();
    } else {
      Low();
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(NormalizationProbe);
};

}  // namespace warps

#endif  // WARPS_DRIVERS_NORMALIZATION_PROBE_H_
