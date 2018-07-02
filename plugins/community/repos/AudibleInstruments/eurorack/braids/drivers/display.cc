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
// Driver for 4x14-segments display.

#include "braids/drivers/display.h"

#include <string.h>

#include "braids/resources.h"

namespace braids {

const uint16_t kPinClk = GPIO_Pin_11;
const uint16_t kPinEnable = GPIO_Pin_10;
const uint16_t kPinData = GPIO_Pin_1;

const uint16_t kCharacterEnablePins[] = {
  GPIO_Pin_5,
  GPIO_Pin_6,
  GPIO_Pin_7,
  GPIO_Pin_8
};

void Display::Init() {
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = kPinClk;
  gpio_init.GPIO_Pin |= kPinEnable;
  gpio_init.GPIO_Pin |= kPinData;
  gpio_init.GPIO_Pin |= kCharacterEnablePins[0];
  gpio_init.GPIO_Pin |= kCharacterEnablePins[1];
  gpio_init.GPIO_Pin |= kCharacterEnablePins[2];
  gpio_init.GPIO_Pin |= kCharacterEnablePins[3];
  
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);

  GPIOB->BSRR = kPinEnable;
  active_position_ = 0;
  brightness_pwm_cycle_ = 0;
  brightness_ = 3;
  memset(buffer_, ' ', kDisplayWidth);
}

void Display::Refresh() {
  if (brightness_pwm_cycle_ <= brightness_) {
    GPIOB->BRR = kCharacterEnablePins[active_position_];
    active_position_ = (active_position_ + 1) % kDisplayWidth;
    Shift14SegmentsWord(chr_characters[
        static_cast<uint8_t>(buffer_[active_position_])]);
    GPIOB->BSRR = kCharacterEnablePins[active_position_];
  } else {
    GPIOB->BRR = kCharacterEnablePins[active_position_];
  }
  brightness_pwm_cycle_ = (brightness_pwm_cycle_ + 1) % kBrightnessLevels;
}

void Display::Print(const char* s) {
  strncpy(buffer_, s, kDisplayWidth);
}

void Display::Shift14SegmentsWord(uint16_t data) {
  GPIOB->BRR = kPinEnable;
  for (uint16_t i = 0; i < 16; ++i) {
    GPIOB->BRR = kPinClk;
    if (data & 1) {
      GPIOB->BSRR = kPinData;
    } else {
      GPIOB->BRR = kPinData;
    }
    data >>= 1;
    GPIOB->BSRR = kPinClk;
  }
  GPIOB->BSRR = kPinEnable;
}

}  // namespace braids
