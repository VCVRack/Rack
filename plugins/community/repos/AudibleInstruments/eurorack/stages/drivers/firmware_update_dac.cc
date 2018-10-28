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
// Lightweight DAC driver used for the firmware update procedure.
// Initializes the I2S port as SPI, and relies on a timer for clock generation.

#include "stages/drivers/firmware_update_dac.h"

namespace stages {

/* static */
FirmwareUpdateDac* FirmwareUpdateDac::instance_;

void FirmwareUpdateDac::Init(int sample_rate) {
  instance_ = this;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

  // Initialize SS pin.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOA, &gpio_init);
  
  // Initialize MOSI and SCK pins.
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_6);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_6);
  GPIO_Init(GPIOC, &gpio_init);
  
  // Initialize SPI.
  SPI_InitTypeDef spi_init;
  spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_DataSize = SPI_DataSize_16b;
  spi_init.SPI_CPOL = SPI_CPOL_High;
  spi_init.SPI_CPHA = SPI_CPHA_1Edge;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
  spi_init.SPI_CRCPolynomial = 7;
  SPI_Init(SPI3, &spi_init);
  SPI_Cmd(SPI3, ENABLE);
  
  // Initialize timer.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  TIM_TimeBaseInitTypeDef timer_init;
  timer_init.TIM_Period = F_CPU / int(sample_rate);
  timer_init.TIM_Prescaler = 0;
  timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer_init.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timer_init);
  TIM_Cmd(TIM2, ENABLE);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 2.2 priority split.
  NVIC_InitTypeDef timer_interrupt;
  timer_interrupt.NVIC_IRQChannel = TIM2_IRQn;
  timer_interrupt.NVIC_IRQChannelPreemptionPriority = 0;
  timer_interrupt.NVIC_IRQChannelSubPriority = 0;
  timer_interrupt.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&timer_interrupt);

  GPIOA->BSRR = GPIO_Pin_15;
  GPIOA->BRR = GPIO_Pin_15;
  SPI3->DR = 0x090a;
  Wait<64>();
  SPI3->DR = 0x0000;
}

}  // namespace stages

extern "C" {

void TIM2_IRQHandler() {
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    stages::FirmwareUpdateDac::GetInstance()->NextSample();
  }
}

}