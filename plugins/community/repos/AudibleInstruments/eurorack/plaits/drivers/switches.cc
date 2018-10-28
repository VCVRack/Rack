// Copyright 2016 Olivier Gillet.
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
// Driver for the 2 switches.

#include "plaits/drivers/switches.h"

#include <algorithm>

namespace plaits {

using namespace std;

struct SwitchDefinition {
  GPIO_TypeDef* gpio;
  uint16_t pin;
};

const SwitchDefinition switch_definitions[] = {
  { GPIOB, GPIO_Pin_7 },
  { GPIOB, GPIO_Pin_6 },
};

void Switches::Init() {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_IN;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
  
  for (int i = 0; i < SWITCH_LAST; ++i) {
    const SwitchDefinition& definition = switch_definitions[i];
    gpio_init.GPIO_Pin = definition.pin;
    GPIO_Init(definition.gpio, &gpio_init);
  }
  fill(&switch_state_[0], &switch_state_[SWITCH_LAST], 0xff);
}

void Switches::Debounce() {
  for (int i = 0; i < SWITCH_LAST; ++i) {
    const SwitchDefinition& definition = switch_definitions[i];
    switch_state_[i] = (switch_state_[i] << 1) | \
        GPIO_ReadInputDataBit(definition.gpio, definition.pin);
  }
}

}  // namespace plaits
