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
// System level initialization.

#include "peaks/drivers/system.h"

#include <stm32f10x_conf.h>

namespace peaks {

void System::Init(uint32_t timer_period, bool application) {
  SystemInit();
  
  if (application) {
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
  }

  RCC_APB2PeriphClockCmd(
      RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
      RCC_APB2Periph_ADC1 |
      RCC_APB2Periph_TIM1 |
      RCC_APB2Periph_USART1 |
      RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(
      RCC_APB1Periph_SPI2 |
      RCC_APB1Periph_TIM3, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
  
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);  // 9 MHz DAC clock

  TIM_TimeBaseInitTypeDef timer_init;
  timer_init.TIM_Period = timer_period;
  timer_init.TIM_Prescaler = 0;
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_InternalClockConfig(TIM1);
  TIM_TimeBaseInit(TIM1, &timer_init);
  TIM_Cmd(TIM1, ENABLE);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 2.2 priority split.
  
  // DAC interrupt is given highest priority
  NVIC_InitTypeDef timer_interrupt;
  timer_interrupt.NVIC_IRQChannel = TIM1_UP_IRQn;
  timer_interrupt.NVIC_IRQChannelPreemptionPriority = 0;
  timer_interrupt.NVIC_IRQChannelSubPriority = 0;
  timer_interrupt.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&timer_interrupt);
}

void System::StartTimers() {
  SysTick_Config(F_CPU / 1000);
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);  
}

}  // namespace peaks
