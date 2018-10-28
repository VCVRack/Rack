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
// Driver for DAC.

#include "marbles/drivers/dac.h"

#include <algorithm>

namespace marbles {

/* static */
Dac* Dac::instance_;

void Dac::Init(int sample_rate, size_t block_size) {
  instance_ = this;
  block_size_ = block_size;
  callback_ = NULL;
  InitializeGPIO();
  InitializeAudioInterface(sample_rate);
  InitializeDMA(block_size);
}

void Dac::InitializeGPIO() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // Initialize SS pin.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOA, &gpio_init);
  
  // Initialize MOSI and SCK pins.
  gpio_init.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
  GPIO_Init(GPIOC, &gpio_init);
  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);
}

void Dac::InitializeAudioInterface(int sample_rate) {
  RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
  RCC_PLLI2SCmd(DISABLE);
  // Best results for multiples of 32kHz.
  RCC_PLLI2SConfig(258, 3);
  RCC_PLLI2SCmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  SPI_I2S_DeInit(SPI3);
  I2S_InitTypeDef i2s_init;
  i2s_init.I2S_Mode = I2S_Mode_MasterTx;
  i2s_init.I2S_Standard = I2S_Standard_PCMShort;
  i2s_init.I2S_DataFormat = I2S_DataFormat_32b;
  i2s_init.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
  i2s_init.I2S_AudioFreq = sample_rate * kNumDacChannels >> 1;
  i2s_init.I2S_CPOL = I2S_CPOL_Low;
  I2S_Init(SPI3, &i2s_init);
  I2S_Cmd(SPI3, ENABLE);
}

void Dac::InitializeDMA(size_t block_size) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  DMA_Cmd(DMA1_Stream5, DISABLE);
  DMA_DeInit(DMA1_Stream5);

  DMA_InitTypeDef dma_init;
  dma_init.DMA_Channel = DMA_Channel_0;
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(SPI3->DR);
  dma_init.DMA_Memory0BaseAddr = (uint32_t)(&tx_dma_buffer_[0]);
  dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dma_init.DMA_BufferSize = 2 * block_size * kNumDacChannels * 2;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &dma_init);

  // Enable the interrupts.
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);
    
  // Enable the IRQ.
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  // Start DMA from/to codec.
  SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
}

void Dac::Start(FillBufferCallback callback) {
  callback_ = callback;
  DMA_Cmd(DMA1_Stream5, ENABLE);
}

void Dac::Stop() {
  DMA_Cmd(DMA1_Stream5, DISABLE);
}

void Dac::Fill(size_t offset) {
  // Fill the buffer.
  IOBuffer::Slice slice = (*callback_)(block_size_);
  uint16_t* p = &tx_dma_buffer_[offset * block_size_ * kNumDacChannels * 2];
  for (size_t i = 0; i < block_size_; ++i) {
    for (size_t j = 0; j < kNumDacChannels; ++j) {
      uint16_t sample = slice.block->cv_output[j][slice.frame_index + i];
      *p++ = 0x1000 | (j << 9) | (sample >> 8);
      *p++ = sample << 8;
    }
  }
}

}  // namespace marbles

extern "C" {

void DMA1_Stream5_IRQHandler(void) {
  uint32_t flags = DMA1->HISR;
  DMA1->HIFCR = DMA_FLAG_TCIF5 | DMA_FLAG_HTIF5;
  if (flags & DMA_FLAG_TCIF5) {
    marbles::Dac::GetInstance()->Fill(1);
  } else if (flags & DMA_FLAG_HTIF5) {
    marbles::Dac::GetInstance()->Fill(0);
  }
}
  
}
