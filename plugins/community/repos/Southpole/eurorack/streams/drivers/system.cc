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

#include "streams/drivers/system.h"

#include <stm32f10x_conf.h>

namespace streams {

void System::Init(bool application) {
  SystemInit();
  if (application) {
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
  }
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 2.2 priority split.
  
  if (application) {
    NVIC_InitTypeDef adc_interrupt;
    adc_interrupt.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    adc_interrupt.NVIC_IRQChannelPreemptionPriority = 0;
    adc_interrupt.NVIC_IRQChannelSubPriority = 0;
    adc_interrupt.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&adc_interrupt);
  } else {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef timer_init;
    timer_init.TIM_Period = F_CPU / 48000 - 1;
    timer_init.TIM_Prescaler = 0;
    timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
    timer_init.TIM_CounterMode = TIM_CounterMode_Up;
    timer_init.TIM_RepetitionCounter = 0;
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInit(TIM2, &timer_init);
    TIM_Cmd(TIM2, ENABLE);
    
    NVIC_InitTypeDef timer_interrupt;
    timer_interrupt.NVIC_IRQChannel = TIM2_IRQn;
    timer_interrupt.NVIC_IRQChannelPreemptionPriority = 0;
    timer_interrupt.NVIC_IRQChannelSubPriority = 0;
    timer_interrupt.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&timer_interrupt);
  }
}

void System::StartTimers() {
  SysTick_Config(F_CPU / 4000);
}

}  // namespace streams
