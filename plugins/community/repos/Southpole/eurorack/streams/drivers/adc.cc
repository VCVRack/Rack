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
// Driver for DAC.

#include "streams/drivers/adc.h"

#include <stm32f10x_conf.h>
#include <string.h>

namespace streams {

/* static */
uint8_t Adc::pots_sequence_[kNumPots] = {
  ADC_Channel_2,
  ADC_Channel_3,
  ADC_Channel_1,
  ADC_Channel_0
};

/* static */
Adc* Adc::instance_;

void Adc::Init(bool single_channel, CvProcessingCallback callback) {
  DMA_InitTypeDef dma_init;
  ADC_InitTypeDef adc_init;
  GPIO_InitTypeDef gpio_init;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 12 MHz DAC clock
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &gpio_init);

  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &gpio_init);
  
  // ADC 1 will be used in DMA mode to scan the CV inputs.
  
  // Use DMA to automatically copy ADC data register to values_ buffer.
  DMA_DeInit(DMA1_Channel1);
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dma_init.DMA_MemoryBaseAddr = (uint32_t)&cvs_[0];
  dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
  dma_init.DMA_BufferSize = single_channel ? 1 : kNumCVs;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &dma_init);
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  ADC_DeInit(ADC1);
  adc_init.ADC_Mode = ADC_Mode_Independent;
  adc_init.ADC_ScanConvMode = ENABLE;
  adc_init.ADC_ContinuousConvMode = ENABLE;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  adc_init.ADC_DataAlign = ADC_DataAlign_Left;
  adc_init.ADC_NbrOfChannel = single_channel ? 1 : kNumCVs;
  ADC_Init(ADC1, &adc_init);
  
  if (single_channel) {
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
  } else {
    // Sample rate: 31.09 kHz
    // 72000 / 6 / (12.5 * 6 + 71.5 * 2 + 55.5 * 2 + 28.5 * 2)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_28Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_71Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 5, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_28Cycles5);
  }
  
  if (callback) {
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  }
  
  ADC_DeInit(ADC2);
  adc_init.ADC_Mode = ADC_Mode_Independent;
  adc_init.ADC_ScanConvMode = DISABLE;
  adc_init.ADC_ContinuousConvMode = DISABLE;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  adc_init.ADC_DataAlign = ADC_DataAlign_Left;
  adc_init.ADC_NbrOfChannel = 1;
  ADC_Init(ADC2, &adc_init);
  
  ADC_Cmd(ADC1, ENABLE);
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));
  
  ADC_Cmd(ADC2, ENABLE);
  ADC_ResetCalibration(ADC2);
  while (ADC_GetResetCalibrationStatus(ADC2));
  ADC_StartCalibration(ADC2);
  while (ADC_GetCalibrationStatus(ADC2));
  
  pot_index_ = kNumPots - 1;
  last_read_pot_ = 0;
  instance_ = this;
  callback_ = callback;
  ScanPots();
}

void Adc::Start() {
  ADC_DMACmd(ADC1, ENABLE);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void Adc::DeInit() {
  ADC_SoftwareStartConvCmd(ADC1, DISABLE);
  ADC_DMACmd(ADC1, DISABLE);
  ADC_Cmd(ADC1, DISABLE);
}

void Adc::ScanPots() {
  pots_[pot_index_] = ADC2->DR;
  last_read_pot_ = pot_index_;
  ++pot_index_;
  if (pot_index_ == kNumPots) {
    pot_index_ = 0;
  }
  
  ADC_RegularChannelConfig(
      ADC2,
      pots_sequence_[pot_index_],
      1,
      ADC_SampleTime_239Cycles5);
  ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

extern "C" {

void DMA1_Channel1_IRQHandler(void) {
  if (DMA_GetITStatus(DMA1_IT_TC1) == SET) {
    DMA_ClearITPendingBit(DMA1_IT_GL1);
    Adc::GetInstance()->Callback();
  }
}

}

}  // namespace streams
