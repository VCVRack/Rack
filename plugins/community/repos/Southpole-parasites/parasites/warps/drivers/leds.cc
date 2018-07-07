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
// Driver for the tricolor LED under the big knob, and the bicolor osc LED.

#include "warps/drivers/leds.h"

#include <algorithm>

namespace warps {
  
using namespace std;

const uint32_t kPwmFrequency = 140624;
const uint16_t kPwmResolution = 8;
  
void Leds::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  TIM_TimeBaseInitTypeDef timer_init;
  TIM_OCInitTypeDef output_compare_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gpio_init);
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);
  
  TIM_TimeBaseStructInit(&timer_init);
  timer_init.TIM_Period = (1 << kPwmResolution) - 1;
  timer_init.TIM_Prescaler = 0;  // Switch fast, die young.
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &timer_init);
  
  TIM_OCStructInit(&output_compare_init);
  output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
  output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
  output_compare_init.TIM_Pulse = 0x00;
  output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
  output_compare_init.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OC1Init(TIM4, &output_compare_init);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_OC2Init(TIM4, &output_compare_init);
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_OC3Init(TIM4, &output_compare_init);
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
  TIM_ARRPreloadConfig(TIM4, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
  
  Clear();
  
  osc_pwm_counter_ = 0;
}

void Leds::Write() {
  TIM4->CCR3 = main_red_ >> (8 - kPwmResolution);
  TIM4->CCR1 = main_green_ >> (8 - kPwmResolution);
  TIM4->CCR2 = main_blue_ >> (8 - kPwmResolution);
  osc_pwm_counter_ += 16;
  GPIO_WriteBit(GPIOC, GPIO_Pin_4, static_cast<BitAction>(
      osc_red_ == 255 || osc_red_ > osc_pwm_counter_));
  GPIO_WriteBit(GPIOC, GPIO_Pin_5, static_cast<BitAction>(
      osc_green_ == 255 || osc_green_ > osc_pwm_counter_));
}

void Leds::Clear() {
  main_red_ = 0;
  main_green_ = 0;
  main_blue_ = 0;
  osc_red_ = 0;
  osc_green_ = 0;
}

}  // namespace warps
