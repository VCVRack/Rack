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
// Driver for the status LEDs.

#include "peaks/drivers/leds.h"

#include <stm32f10x_conf.h>

namespace peaks {

const uint32_t kPwmFrequency = 125000;
const uint16_t kPwmResolution = 6;  // bits

void Leds::Init() {
  GPIO_InitTypeDef gpio_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &gpio_init);

  gpio_init.GPIO_Pin = GPIO_Pin_10;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  
  TIM_TimeBaseInitTypeDef timer_init;
  timer_init.TIM_Period = (1 << kPwmResolution) - 1;
  timer_init.TIM_Prescaler = F_CPU / kPwmFrequency / (1 << kPwmResolution) - 1;
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_InternalClockConfig(TIM3);
  TIM_TimeBaseInit(TIM3, &timer_init);
  
  TIM_OCInitTypeDef output_compare_init;
  output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
  output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
  output_compare_init.TIM_Pulse = 0x00;
  output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
  
  TIM_OC3Init(TIM3, &output_compare_init);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_OC4Init(TIM3, &output_compare_init);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
  
  TIM_Cmd(TIM3, ENABLE);
}

void Leds::Write() {
  TIM_SetCompare3(TIM3, levels_[1] >> (8 - kPwmResolution));
  TIM_SetCompare4(TIM3, levels_[0] >> (8 - kPwmResolution));
  GPIO_WriteBit(GPIOB, GPIO_Pin_10, static_cast<BitAction>(twin_mode_));
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, static_cast<BitAction>(function_ & 1));
  GPIO_WriteBit(GPIOA, GPIO_Pin_5, static_cast<BitAction>(function_ & 2));
  GPIO_WriteBit(GPIOA, GPIO_Pin_6, static_cast<BitAction>(function_ & 4));
  GPIO_WriteBit(GPIOA, GPIO_Pin_7, static_cast<BitAction>(function_ & 8));
}

}  // namespace peaks
