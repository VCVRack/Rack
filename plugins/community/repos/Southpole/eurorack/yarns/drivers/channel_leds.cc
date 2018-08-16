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
// Driver for the 4 channels LEDs.

#include "yarns/drivers/channel_leds.h"

#include <stm32f10x_conf.h>

namespace yarns {

void ChannelLeds::Init() {
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_8;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_14;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  pwm_counter_ = 0;
  std::fill(&brightness_[0], &brightness_[4], 0);
}

void ChannelLeds::Write() {
  pwm_counter_ += 16;
  
  GPIO_WriteBit(GPIOA, GPIO_Pin_12,
                static_cast<BitAction>(brightness_[0] > pwm_counter_));
  GPIO_WriteBit(GPIOA, GPIO_Pin_11,
                static_cast<BitAction>(brightness_[1] > pwm_counter_));
  GPIO_WriteBit(GPIOA, GPIO_Pin_8,
                static_cast<BitAction>(brightness_[2] > pwm_counter_));
  GPIO_WriteBit(GPIOB, GPIO_Pin_14,
                static_cast<BitAction>(brightness_[3] > pwm_counter_));
}

}  // namespace yarns
