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

#include "streams/drivers/leds.h"

#include <algorithm>

#include <stm32f10x_conf.h>

namespace streams {

using namespace std;

const uint16_t kPinClk = GPIO_Pin_11;
const uint16_t kPinData = GPIO_Pin_10;
const uint16_t kPinEnable = GPIO_Pin_7;

void Leds::Init() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = kPinClk | kPinEnable | kPinData;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
  GPIOB->BSRR = kPinEnable;

  Clear();
  pwm_counter_ = 0;
  
  WriteMcp23s17(0 /* IODIRA */, 0);
  WriteMcp23s17(1 /* IODIRB */, 0);
  WriteMcp23s17(2 /* IOPOLA */, 0);
  WriteMcp23s17(3 /* IOPOLB */, 0);
}

void Leds::ShiftOut(uint8_t byte) {
  for (uint8_t i = 0; i < 8; ++i) {
    GPIOB->BRR = kPinClk;
    if (byte & 0x80) {
      GPIOB->BSRR = kPinData;
    } else {
      GPIOB->BRR = kPinData;
    }
    byte <<= 1;
    GPIOB->BSRR = kPinClk;
  }
}

void Leds::WriteMcp23s17(uint8_t address, uint8_t data) {
  GPIOB->BRR = kPinEnable;
  ShiftOut(0x40);
  ShiftOut(address);
  ShiftOut(data);
  GPIOB->BSRR = kPinEnable;
}

void Leds::Clear() {
  fill(&red_[0], &red_[kNumLeds], 0);
  fill(&green_[0], &green_[kNumLeds], 0);
}

void Leds::Write() {
  pwm_counter_ += 32;
  uint16_t data = 0;
  for (uint16_t i = 0; i < kNumLeds; ++i) {
    uint8_t index = i < 4 ? i + 4 : 7 - i;
    data <<= 2;
    if (red_[index] && red_[index] >= pwm_counter_) {
      data |= 0x1;
    }
    if (green_[index] && green_[index] >= pwm_counter_) {
      data |= 0x2;
    }
  }
  WriteMcp23s17(0x12 /* GPIOA */, data >> 8);
  WriteMcp23s17(0x13 /* GPIOB */, data & 0xff);
}

}  // namespace streams
