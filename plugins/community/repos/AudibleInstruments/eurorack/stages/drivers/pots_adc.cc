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
// Drivers for the 12-bit SAR ADC scanning pots and sliders.

#include "stages/drivers/pots_adc.h"

#include <stm32f37x_conf.h>

namespace stages {

/* static */
uint8_t PotsAdc::mux_address_to_pot_index_[kNumMuxAddresses] = {
  0xff,
  2,
  4,
  1,
  3,
  0,
  0xff,
  5,
};

/* static */
uint8_t PotsAdc::mux_address_to_slider_index_[kNumMuxAddresses] = {
  3,
  2,
  1,
  4,
  0,
  0xff,
  5,
  0xff,
};

void PotsAdc::Init() {
  // Enable ADC clock.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
  
  // Enable GPIO clock.
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  // Enable DMA1 clock.
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  DMA_InitTypeDef dma_init;
  ADC_InitTypeDef adc_init;
  GPIO_InitTypeDef gpio_init;
  
  // Configure the two analog inputs for the pots and sliders muxed signals.
  gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOA, &gpio_init);

  // Configure the address lines for the MUX.
  gpio_init.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gpio_init);
  GPIO_ResetBits(GPIOA, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);
  
  // Use DMA to automatically copy ADC data register to values_ buffer.
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dma_init.DMA_MemoryBaseAddr = (uint32_t)&adc_values_[0];
  dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
  dma_init.DMA_BufferSize = 2;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &dma_init);
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  // Init ADC1
  adc_init.ADC_ScanConvMode = ENABLE;
  adc_init.ADC_ContinuousConvMode = DISABLE;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  adc_init.ADC_DataAlign = ADC_DataAlign_Left;
  adc_init.ADC_NbrOfChannel = 2;
  ADC_Init(ADC1, &adc_init);
  
  // Sample rate: 17.85 kHz
  // 72000 / 8 / (12.5 * 2 + 239.5 * 2)
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_239Cycles5);
  
  // Enable and calibrate ADC1.
  ADC_Cmd(ADC1, ENABLE);
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));
  ADC_DMACmd(ADC1, ENABLE);
  
  mux_address_ = 0;
  conversion_done_ = true;
  pot_index_ = 0xff;
  slider_index_ = 0xff;
}

void PotsAdc::DeInit() {
  ADC_DMACmd(ADC1, DISABLE);
  ADC_Cmd(ADC1, DISABLE);
}

void PotsAdc::Convert() {
  if (conversion_done_) {
    uint8_t pot_index = mux_address_to_pot_index_[mux_address_];
    if (pot_index != 0xff) {
      values_[ADC_GROUP_POT + pot_index] = adc_values_[0];
    }
    pot_index_ = pot_index;
    
    uint8_t slider_index = mux_address_to_slider_index_[mux_address_];
    if (slider_index != 0xff) {
      // Due to the odd PCB layout, some sliders are rotated 180 degrees.
      // We need to flip the readout for those.
      uint16_t mask = slider_index == 0 || slider_index == 5 ? 0xffff : 0x0000;
      values_[ADC_GROUP_SLIDER + slider_index] = adc_values_[1] ^ mask;
    }
    slider_index_ = slider_index;
    
    mux_address_ = (mux_address_ + 1) % 8;

    // GPIO_WriteBit(GPIOA, GPIO_Pin_3, static_cast<BitAction>(mux_address_ & 4));
    // GPIO_WriteBit(GPIOA, GPIO_Pin_4, static_cast<BitAction>(mux_address_ & 2));
    // GPIO_WriteBit(GPIOA, GPIO_Pin_5, static_cast<BitAction>(mux_address_ & 1));
    if (mux_address_ & 4) {
      GPIOA->BSRR = GPIO_Pin_3;
    } else {
      GPIOA->BRR = GPIO_Pin_3;
    }
    if (mux_address_ & 2) {
      GPIOA->BSRR = GPIO_Pin_4;
    } else {
      GPIOA->BRR = GPIO_Pin_4;
    }
    if (mux_address_ & 1) {
      GPIOA->BSRR = GPIO_Pin_5;
    } else {
      GPIOA->BRR = GPIO_Pin_5;
    }
  } else {
    // ADC_SoftwareStartConv(ADC1);
    ADC1->CR2 |= (uint32_t)(ADC_CR2_SWSTART | ADC_CR2_EXTTRIG);
  }
  conversion_done_ = !conversion_done_;
}

}  // namespace stages
