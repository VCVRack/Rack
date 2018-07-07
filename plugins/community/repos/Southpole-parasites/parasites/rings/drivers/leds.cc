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
// Driver for the 2 LEDs.

#include "rings/drivers/leds.h"

namespace rings {
  
void Leds::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio_init);
  
  red_[0] = green_[0] = red_[1] = green_[1] = 0;
}

void Leds::Write() {
  GPIO_WriteBit(GPIOC, GPIO_Pin_12, static_cast<BitAction>(red_[0]));
  GPIO_WriteBit(GPIOC, GPIO_Pin_11, static_cast<BitAction>(green_[0]));
  GPIO_WriteBit(GPIOC, GPIO_Pin_0, static_cast<BitAction>(red_[1]));
  GPIO_WriteBit(GPIOC, GPIO_Pin_10, static_cast<BitAction>(green_[1]));
}

}  // namespace rings
