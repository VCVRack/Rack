// Copyright 2017 Olivier Gillet.
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
// Drivers for the sliders and UI LEDs.

#include "stages/drivers/leds.h"

#include <algorithm>

#include <stm32f37x_conf.h>

namespace stages {

using namespace std;

struct SliderLedDefinition {
  GPIO_TypeDef* gpio;
  uint16_t pin;
};

const uint16_t kPinClk = GPIO_Pin_6;
const uint16_t kPinEnable = GPIO_Pin_7;
const uint16_t kPinData = GPIO_Pin_5;

const SliderLedDefinition slider_led_definition[kNumLEDs] = {
  { GPIOA, GPIO_Pin_12 },
  { GPIOC, GPIO_Pin_13 },
  { GPIOC, GPIO_Pin_14 },
  { GPIOC, GPIO_Pin_15 },
  { GPIOA, GPIO_Pin_2 },
  { GPIOB, GPIO_Pin_2 }
};


/* static */
const uint16_t red_bitmask[] = {
  1 << 5, 1 << 7, 1 << 2, 1 << 9, 1 << 10, 1 << 12
};

/* static */
const uint16_t green_bitmask[] = {
  1 << 4, 1 << 6, 1 << 3, 1 << 8, 1 << 11, 1 << 13
};


void Leds::Init() {
  // GPIOs for sliders.
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  for (int i = 0; i < kNumLEDs; ++i) {
    const SliderLedDefinition& d = slider_led_definition[i];
    gpio_init.GPIO_Pin = d.pin;
    GPIO_Init(d.gpio, &gpio_init);
  }
  
  // GPIOs for UI leds.
  gpio_init.GPIO_Pin = kPinClk | kPinEnable | kPinData;
  GPIO_Init(GPIOB, &gpio_init);
  
  Clear();
}

void Leds::Clear() {
  fill(&colors_[0], &colors_[LED_GROUP_SLIDER + kNumLEDs], LED_COLOR_OFF);
}

void Leds::Write() {
  uint16_t leds_data = 0;
  
  for (int i = 0; i < kNumLEDs; ++i) {
    const SliderLedDefinition& d = slider_led_definition[i];
    if ((colors_[LED_GROUP_SLIDER + i] & 0x008000) >> 15) {
      d.gpio->BSRR = d.pin;
    } else {
      d.gpio->BRR = d.pin;
    }
    
    if (colors_[LED_GROUP_UI + i] & 0x800000) {
      leds_data |= red_bitmask[i];
    }
    if (colors_[LED_GROUP_UI + i] & 0x008000) {
      leds_data |= green_bitmask[i];
    }
  }
  
  // GPIO_WriteBit(GPIOB, kPinEnable, Bit_RESET);
  GPIOB->BRR = kPinEnable;

  for (uint16_t i = 0; i < 16; ++i) {
    // GPIO_WriteBit(GPIOB, kPinClk, Bit_RESET);
    GPIOB->BRR = kPinClk;
    
    if (leds_data & 0x8000) {
      // GPIO_WriteBit(GPIOB, kPinData, Bit_SET);
      GPIOB->BSRR = kPinData;
    } else {
      // GPIO_WriteBit(GPIOB, kPinData, Bit_RESET);
      GPIOB->BRR = kPinData;
    }
    leds_data <<= 1;

    // GPIO_WriteBit(GPIOB, kPinClk, Bit_SET);
    GPIOB->BSRR = kPinClk;
  }
  // GPIO_WriteBit(GPIOB, kPinEnable, Bit_SET);
  GPIOB->BSRR = kPinEnable;
}

}  // namespace stages
