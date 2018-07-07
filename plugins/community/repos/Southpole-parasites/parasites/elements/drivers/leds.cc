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
// Driver for the 3 LEDs.

#include "elements/drivers/leds.h"

#include <algorithm>

namespace elements {
  
using namespace std;

const uint32_t kPwmFrequency = 140624;
const uint16_t kPwmResolution = 8;

void Leds::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  TIM_TimeBaseInitTypeDef timer_init;
  TIM_OCInitTypeDef output_compare_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_7;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gpio_init);
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);
  
  TIM_TimeBaseStructInit(&timer_init);
  timer_init.TIM_Period = (1 << kPwmResolution) - 1;
  timer_init.TIM_Prescaler = 0;  // Switch fast, die young.
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &timer_init);
  
  TIM_OCStructInit(&output_compare_init);
  output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
  output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
  output_compare_init.TIM_OutputNState = TIM_OutputNState_Disable;
  output_compare_init.TIM_Pulse = 0x00;
  output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
  output_compare_init.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OC1Init(TIM1, &output_compare_init);
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
  
  TIM_OCStructInit(&output_compare_init);
  output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
  output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
  output_compare_init.TIM_Pulse = 0x00;
  output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC4Init(TIM1, &output_compare_init);
  TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM1, ENABLE);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);
  TIM_Cmd(TIM1, ENABLE);
}

void Leds::Write() {
  GPIO_WriteBit(GPIOC, GPIO_Pin_7, static_cast<BitAction>(gate_));
  TIM1->CCR1 = exciter_;
  TIM1->CCR4 = resonator_;
}

}  // namespace elements
