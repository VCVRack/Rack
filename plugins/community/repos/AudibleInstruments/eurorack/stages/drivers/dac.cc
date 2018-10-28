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
// Driver for the DAC.

#include "stages/drivers/dac.h"

#include <algorithm>

namespace stages {
  
/* static */
int Dac::channel_selection_bits_[kNumChannels] = {
  0x0300 | (3 << 4),
  0x0300 | (1 << 4),
  0x0300 | (6 << 4),
  0x0300 | (2 << 4),
  0x0300 | (0 << 4),
  0x0300 | (4 << 4),
};

/* static */
Dac* Dac::instance_;
  
void Dac::Init(int sample_rate, size_t block_size) {
  instance_ = this;
  block_size_ = block_size;
  callback_ = NULL;
  first_frame_ = true;
  
  InitializeGPIO();
  InitializeAudioInterface(sample_rate);
  InitializeDMA(block_size);
}

void Dac::InitializeGPIO() {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  // Initialize SS pin.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOA, &gpio_init);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_6);
  
  gpio_init.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
  GPIO_Init(GPIOC, &gpio_init);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_6);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_6);
}

void Dac::InitializeAudioInterface(int sample_rate) {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  SPI_I2S_DeInit(SPI3);
  I2S_InitTypeDef i2s_init;
  i2s_init.I2S_Mode = I2S_Mode_MasterTx;
  i2s_init.I2S_Standard = I2S_Standard_Phillips;
  i2s_init.I2S_AudioFreq = sample_rate * kNumChannels;

  i2s_init.I2S_DataFormat = I2S_DataFormat_32b;
  i2s_init.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
  i2s_init.I2S_CPOL = I2S_CPOL_Low;
  I2S_Init(SPI3, &i2s_init);
  I2S_Cmd(SPI3, ENABLE);
}

void Dac::InitializeDMA(size_t block_size) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  
  DMA_Cmd(DMA2_Channel2, DISABLE);
  DMA_DeInit(DMA2_Channel2);

  DMA_InitTypeDef dma_init;
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(SPI3->DR);
  dma_init.DMA_MemoryBaseAddr = (uint32_t)(&tx_dma_buffer_[0]);
  dma_init.DMA_DIR = DMA_DIR_PeripheralDST;
  dma_init.DMA_BufferSize = 2 * block_size * kFrameSize;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel2, &dma_init);
  
  // Enable the interrupts: half transfer and transfer complete.
	DMA_ITConfig(DMA2_Channel2, DMA_IT_TC | DMA_IT_HT, ENABLE);
  
  // Enable the IRQ.
  NVIC_EnableIRQ(DMA2_Channel2_IRQn);
  
  SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
}

void Dac::Start(FillBufferCallback callback) {
  callback_ = callback;
  DMA_Cmd(DMA2_Channel2, ENABLE);
  first_frame_ = true;
}

void Dac::Stop() {
  DMA_Cmd(DMA2_Channel2, DISABLE);
}

void Dac::Fill(size_t offset) {
  uint16_t* p = &tx_dma_buffer_[offset * block_size_ * kFrameSize];
  if (first_frame_) {
    for (size_t i = 0; i < block_size_; ++i) {
      for (size_t j = 0; j < kNumChannels; ++j) {
        *p++ = 0x090a;
        *p++ = 0x0000;
        *p++ = 0x0000;
        *p++ = 0x0000;
      }
    }
    first_frame_ = false;
  } else {
    IOBuffer::Slice slice = (*callback_)(block_size_);
    for (size_t i = 0; i < block_size_; ++i) {
      for (size_t j = 0; j < kNumChannels; ++j) {
        uint16_t word = slice.block->output[j][slice.frame_index + i];
        *p++ = channel_selection_bits_[j] | (word >> 12);
        *p++ = word << 4;
        *p++ = 0;
        *p++ = 0;
      }
    }
  }
}

}  // namespace stages

extern "C" {

void DMA2_Channel2_IRQHandler() {
  uint32_t flags = DMA2->ISR;
  DMA2->IFCR = DMA2_FLAG_TC2 | DMA2_FLAG_HT2;
  if (flags & DMA2_FLAG_TC2) {
    stages::Dac::GetInstance()->Fill(1);
  } else if (flags & DMA2_FLAG_HT2) {
    stages::Dac::GetInstance()->Fill(0);
  }
}

}