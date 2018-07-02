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

#include "tides/drivers/leds.h"

#include <stm32f10x_conf.h>

namespace tides {

const uint32_t kPwmFrequency = 140624;
const uint16_t kPwmResolution = 9;  // bits

void Leds::Init() {
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  TIM_TimeBaseInitTypeDef timer_init;
  timer_init.TIM_Period = (1 << kPwmResolution) - 1;
  timer_init.TIM_Prescaler = F_CPU / kPwmFrequency / (1 << kPwmResolution) - 1;
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_InternalClockConfig(TIM4);
  TIM_TimeBaseInit(TIM4, &timer_init);
  
  TIM_OCInitTypeDef output_compare_init;
  output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
  output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
  output_compare_init.TIM_Pulse = 0x00;
  output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
  
  TIM_OC1Init(TIM4, &output_compare_init);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC2Init(TIM4, &output_compare_init);
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC3Init(TIM4, &output_compare_init);
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC4Init(TIM4, &output_compare_init);
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
  TIM_Cmd(TIM4, ENABLE);
}

void Leds::Write() {
  TIM_SetCompare1(TIM4, value_g_ >> (16 - kPwmResolution));
  TIM_SetCompare2(TIM4, value_r_ >> (16 - kPwmResolution));
  TIM_SetCompare3(TIM4, rate_g_ >> (16 - kPwmResolution));
  TIM_SetCompare4(TIM4, rate_r_ >> (16 - kPwmResolution));
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, static_cast<BitAction>(mode_r_));
  GPIO_WriteBit(GPIOB, GPIO_Pin_4, static_cast<BitAction>(mode_g_));
}

}  // namespace tides
