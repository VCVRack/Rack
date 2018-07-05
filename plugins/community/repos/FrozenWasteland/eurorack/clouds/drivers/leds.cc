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
// Driver for the 4 bicolor LEDs controlled by a 595.

#include "clouds/drivers/leds.h"

#include <algorithm>

namespace clouds {
  
using namespace std;
  
const uint16_t kPinClk = GPIO_Pin_5;
const uint16_t kPinEnable = GPIO_Pin_4;
const uint16_t kPinData = GPIO_Pin_5;
const uint16_t kPinFreezeLed = GPIO_Pin_9;

void Leds::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = kPinData | kPinFreezeLed;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gpio_init);
  
  gpio_init.GPIO_Pin = kPinClk | kPinEnable;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio_init);
  
  Clear();
  pwm_counter_ = 0;
}

void Leds::Write() {
  // Pack data.
  pwm_counter_ += 32;
  uint8_t data = 0;
  for (uint16_t i = 0; i < 4; ++i) {
    data <<= 2;
    if (red_[i] && red_[i] >= pwm_counter_) {
      data |= 0x1;
    }
    if (green_[i] && green_[i] >= pwm_counter_) {
      data |= 0x2;
    }
  }
  // Shift out.
  GPIO_WriteBit(GPIOC, kPinEnable, Bit_RESET);
  for (uint16_t i = 0; i < 8; ++i) {
    GPIO_WriteBit(GPIOC, kPinClk, Bit_RESET);
    if (data & 0x80) {
      GPIO_WriteBit(GPIOB, kPinData, Bit_SET);
    } else {
      GPIO_WriteBit(GPIOB, kPinData, Bit_RESET);
    }
    data <<= 1;
    GPIO_WriteBit(GPIOC, kPinClk, Bit_SET);
  }
  GPIO_WriteBit(GPIOC, kPinEnable, Bit_SET);
  GPIO_WriteBit(GPIOB, kPinFreezeLed, static_cast<BitAction>(freeze_led_));
}

void Leds::Clear() {
  fill(&red_[0], &red_[4], 0);
  fill(&green_[0], &green_[4], 0);
  freeze_led_ = false;
}

}  // namespace clouds
