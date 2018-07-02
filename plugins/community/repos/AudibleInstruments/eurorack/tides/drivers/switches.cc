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
// Driver for the Mode and Range switches.

#include "tides/drivers/switches.h"

#include <string.h>

namespace tides {

void Switches::Init() {
  memset(&switch_state_, 0xff, sizeof(switch_state_));
  
  GPIO_InitTypeDef gpio_init;

  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &gpio_init);
}

void Switches::Debounce() {
  switch_state_[0] = (switch_state_[0] << 1) | \
      GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
  switch_state_[1] = (switch_state_[1] << 1) | \
      GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
}

}  // namespace tides
