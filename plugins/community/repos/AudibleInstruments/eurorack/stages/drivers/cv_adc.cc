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
// Drivers for the 16-bit SDADC reading the CVs.

#include "stages/drivers/cv_adc.h"

#include <stm32f37x_conf.h>

namespace stages {

struct ChannelConfiguration {
  int map_to;
  uint32_t channel;
  GPIO_TypeDef* gpio;
  uint16_t pin;
};

struct ConverterConfiguration {
  SDADC_TypeDef* sdadc;
  DMA_Channel_TypeDef* dma_channel;
  int num_channels;
  ChannelConfiguration channel[2];
};

const ConverterConfiguration converter_configuration[3] = {
  {
    SDADC1, DMA2_Channel3, 2, {
      { 4, SDADC_Channel_5, GPIOB, GPIO_Pin_1 },
      { 5, SDADC_Channel_6, GPIOB, GPIO_Pin_0 },
    }
  },
  {
    SDADC2, DMA2_Channel4, 2, {
      { 3, SDADC_Channel_7, GPIOE, GPIO_Pin_9 },
      { 2, SDADC_Channel_8, GPIOE, GPIO_Pin_8 }
    }
  },
  {
    SDADC3, DMA2_Channel5, 2, {
      { 1, SDADC_Channel_7, GPIOB, GPIO_Pin_15 },
      { 0, SDADC_Channel_8, GPIOB, GPIO_Pin_14 }
    }
  },
};

void CvAdc::Init() {
  // Power all the SDADCs.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_SDADCAnalogCmd(PWR_SDADCAnalog_1, ENABLE);
  PWR_SDADCAnalogCmd(PWR_SDADCAnalog_2, ENABLE);
  PWR_SDADCAnalogCmd(PWR_SDADCAnalog_3, ENABLE);

  // Enable SDADC clock.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDADC1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDADC2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDADC3, ENABLE);
  RCC_SDADCCLKConfig(RCC_SDADCCLK_SYSCLK_Div8);
  
  // Enable DMA2 clock.
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  
  // Enable GPIO clock.
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
  
  // Init SDADC
  SDADC_VREFSelect(SDADC_VREF_Ext);
  
  DMA_InitTypeDef dma_init;
  GPIO_InitTypeDef gpio_init;
  SDADC_AINStructTypeDef sdadc_ain;
  
  // Fill structures with the settings common to all channels/pins.
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;

  sdadc_ain.SDADC_InputMode = SDADC_InputMode_SEZeroReference;
  sdadc_ain.SDADC_Gain = SDADC_Gain_1;
  sdadc_ain.SDADC_CommonMode = SDADC_CommonMode_VDDA_2;
  sdadc_ain.SDADC_Offset = 0;
  
  int current_channel = 0;
  
  // Configure all SDADCs, all their input channels, and all the DMA channels.
  for (int i = 0; i < 3; ++i) {
    const ConverterConfiguration& config = converter_configuration[i];

    // Wait for SDADC to stabilize.
    SDADC_Cmd(config.sdadc, ENABLE);
    while (SDADC_GetFlagStatus(config.sdadc, SDADC_FLAG_STABIP) == SET);
  
    // Configure GPIO pins.
    for (int j = 0; j < config.num_channels; ++j) {
      gpio_init.GPIO_Pin = config.channel[j].pin;
      GPIO_Init(config.channel[j].gpio, &gpio_init);
    }
    
    // SDADC enters initialization mode.
    SDADC_InitModeCmd(config.sdadc, ENABLE);
    while (SDADC_GetFlagStatus(config.sdadc, SDADC_FLAG_INITRDY) == RESET);
    
    // Configure DMA to read injected values into a slice of the
    // values_ array.
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(config.sdadc->JDATAR);
    dma_init.DMA_MemoryBaseAddr = (uint32_t)(&values_[current_channel]);
    dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_init.DMA_BufferSize = config.num_channels;
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(config.dma_channel, &dma_init);

    // Create a configuration and assign it to all channels used by this SDADC.
    SDADC_AINInit(config.sdadc, SDADC_Conf_0, &sdadc_ain);
    uint32_t channels = 0;
    for (int j = 0; j < config.num_channels; ++j) {
      channel_map_[config.channel[j].map_to] = current_channel++;
      channels |= config.channel[j].channel;
      SDADC_ChannelConfig(
          config.sdadc, config.channel[j].channel, SDADC_Conf_0);
    }
    
    // Select injected channels.
    SDADC_InjectedChannelSelect(config.sdadc, channels);
    
    // Disable continuous mode - the conversions are restarted every time
    // we render a block of samples.
    SDADC_InjectedContinuousModeCmd(config.sdadc, DISABLE);
    
    // Terminate initialization sequence.
    SDADC_CalibrationSequenceConfig(config.sdadc, SDADC_CalibrationSequence_3);
    SDADC_InitModeCmd(config.sdadc, DISABLE);
    while (SDADC_GetFlagStatus(config.sdadc, SDADC_FLAG_INITRDY) == SET);
    
    // Run calibration sequence.
    SDADC_StartCalibration(config.sdadc);
    while (SDADC_GetFlagStatus(config.sdadc, SDADC_FLAG_EOCAL) == RESET);

    // Enable DMA.
    DMA_Cmd(config.dma_channel, ENABLE);
    SDADC_DMAConfig(config.sdadc, SDADC_DMATransfer_Injected, ENABLE);
  }
}

void CvAdc::DeInit() {
  for (int i = 0; i < 3; ++i) {
    const ConverterConfiguration& config = converter_configuration[i];
    SDADC_Cmd(config.sdadc, DISABLE);
    SDADC_DMAConfig(config.sdadc, SDADC_DMATransfer_Injected, DISABLE);
    DMA_Cmd(config.dma_channel, DISABLE);
  }
}

void CvAdc::Convert() {
  // SDADC_SoftwareStartInjectedConv(SDADC1);
  // SDADC_SoftwareStartInjectedConv(SDADC2);
  // SDADC_SoftwareStartInjectedConv(SDADC3);
  SDADC1->CR2 |= (uint32_t)SDADC_CR2_JSWSTART;
  SDADC2->CR2 |= (uint32_t)SDADC_CR2_JSWSTART;
  SDADC3->CR2 |= (uint32_t)SDADC_CR2_JSWSTART;
}

}  // namespace stages
