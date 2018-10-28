// Copyright 2015 Olivier Gillet.
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
// Driver for ADC. ADC1 is used for the 8 pots ; ADC2 for the 8 CV inputs.

#include "marbles/drivers/adc.h"

#include <stm32f4xx_conf.h>

namespace marbles {
  
void Adc::Init(bool single_channel) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
  
  DMA_InitTypeDef dma_init;
  ADC_CommonInitTypeDef adc_common_init;
  ADC_InitTypeDef adc_init;
  GPIO_InitTypeDef gpio_init;
  
  // Initialize A0..A7 (ADC0..ADC7)
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  gpio_init.GPIO_Pin |= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOA, &gpio_init);

  // Initialize B0..B1 (ADC8..ADC9)
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOB, &gpio_init);

  // Initialize C0..C5 (ADC10..ADC11)
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  gpio_init.GPIO_Pin |= GPIO_Pin_4 | GPIO_Pin_5;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOC, &gpio_init);

  // Use DMA to automatically copy ADC data register to values_ buffer.
  dma_init.DMA_Channel = DMA_Channel_0;
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_GROUP_POT];
  dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dma_init.DMA_BufferSize = single_channel ? 1 : ADC_CHANNEL_LAST;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &dma_init);
  DMA_Cmd(DMA2_Stream0, ENABLE);
  
  dma_init.DMA_Channel = DMA_Channel_1;
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC2->DR;
  dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_GROUP_CV];
  DMA_Init(DMA2_Stream2, &dma_init);
  DMA_Cmd(DMA2_Stream2, ENABLE);
  
  adc_common_init.ADC_Mode = ADC_Mode_Independent;
  adc_common_init.ADC_Prescaler = ADC_Prescaler_Div8;
  adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
  ADC_CommonInit(&adc_common_init);
  
  adc_init.ADC_Resolution = ADC_Resolution_12b;
  adc_init.ADC_ScanConvMode = ENABLE;
  adc_init.ADC_ContinuousConvMode = DISABLE;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  adc_init.ADC_DataAlign = ADC_DataAlign_Left;
  adc_init.ADC_NbrOfConversion = single_channel ? 1 : ADC_CHANNEL_LAST;
  ADC_Init(ADC1, &adc_init);
  ADC_Init(ADC2, &adc_init);
  
  // 168M / 2 / 8 / (8 x (144 + 20)) = 8.001kHz.
  if (single_channel) {
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_144Cycles);
  } else {
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_144Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12,3, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2,4, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15,5, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10,6, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 7, ADC_SampleTime_144Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 8, ADC_SampleTime_144Cycles); 
  }
  
  if (single_channel) {
    ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 1, ADC_SampleTime_144Cycles); 
  } else {
    ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 1, ADC_SampleTime_144Cycles);
    ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 2, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 3, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 4, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 5, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC2, ADC_Channel_7, 6, ADC_SampleTime_144Cycles); 
    ADC_RegularChannelConfig(ADC2, ADC_Channel_14,7, ADC_SampleTime_144Cycles);
    ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 8, ADC_SampleTime_144Cycles); 
  }
  
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMACmd(ADC2, ENABLE);
  Convert();
}

void Adc::DeInit() {
  DMA_Cmd(DMA2_Stream0, DISABLE);
  DMA_Cmd(DMA2_Stream2, DISABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC2, DISABLE);
  ADC_Cmd(ADC1, DISABLE);
  ADC_Cmd(ADC2, DISABLE);
  ADC_DMACmd(ADC1, DISABLE);
  ADC_DMACmd(ADC2, DISABLE);
  ADC_DeInit();
}

void Adc::Convert() {
  ADC_SoftwareStartConv(ADC1);
  ADC_SoftwareStartConv(ADC2);
}

}  // namespace marbles
