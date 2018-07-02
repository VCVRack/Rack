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
// Driver for ADC.

#include "peaks/drivers/adc.h"

#include <stm32f10x_conf.h>

namespace peaks {
  
void Adc::Init() {
  DMA_InitTypeDef dma_init;
  ADC_InitTypeDef adc_init;
  GPIO_InitTypeDef gpio_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &gpio_init);
  
  // Use DMA to automatically copy DAC data register to values_ buffer.
  DMA_DeInit(DMA1_Channel1);
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR; 
  dma_init.DMA_MemoryBaseAddr = (uint32_t)&values_[0]; 
  dma_init.DMA_DIR = DMA_DIR_PeripheralSRC; 
  dma_init.DMA_BufferSize = kNumAdcChannels; 
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
  adc_init.ADC_NbrOfChannel = kNumAdcChannels;
  ADC_Init(ADC1, &adc_init);
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_239Cycles5); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 3, ADC_SampleTime_239Cycles5); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 4, ADC_SampleTime_239Cycles5); 
  
  ADC_Cmd(ADC1, ENABLE);
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));

  ADC_DMACmd(ADC1, ENABLE);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

}  // namespace peaks
