// Copyright 2015 Olivier Gillet.
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
// Driver for the debug (timing) pin.

#ifndef RINGS_DRIVERS_DEBUG_PIN_H_
#define RINGS_DRIVERS_DEBUG_PIN_H_

#include "stmlib/stmlib.h"

#ifndef TEST
#include <stm32f4xx_conf.h>
#endif 

namespace rings {

class DebugPin {
 public:
  DebugPin() { }
  ~DebugPin() { }
#ifdef TEST
  static void Init() { }
  static void High() { }
  static void Low() { }
#else
  static void Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_9;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio_init);
  }
  static inline void High() {
    GPIOA->BSRRL = GPIO_Pin_9;
  }
  static inline void Low() {
    GPIOA->BSRRH = GPIO_Pin_9;
  }
#endif
 private:
  DISALLOW_COPY_AND_ASSIGN(DebugPin);
};

#define TIC DebugPin::High();
#define TOC DebugPin::Low();

}  // namespace rings

#endif  // RINGS_DRIVERS_DEBUG_PIN_H_
