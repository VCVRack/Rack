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
// UART driver for agreeable conversations with the neighbors.

#include "stages/drivers/serial_link.h"

namespace stages {

struct DirectionDefinition {
  GPIO_TypeDef* gpio;
  uint16_t tx_rx_pins;
  uint16_t rx_pin_source;
  uint16_t tx_pin_source;

  USART_TypeDef* usart;

  DMA_Channel_TypeDef* tx_dma_channel;
  DMA_Channel_TypeDef* rx_dma_channel;
  uint32_t tx_complete_flag;
  uint32_t rx_complete_flag;
  uint32_t rx_half_complete_flag;
};

const DirectionDefinition direction_definition[] = {
  { GPIOB,
    GPIO_Pin_8 | GPIO_Pin_9,
    GPIO_PinSource8,
    GPIO_PinSource9,
    USART3,
    DMA1_Channel2,
    DMA1_Channel3,
    DMA1_FLAG_TC2,
    DMA1_FLAG_TC3,
    DMA1_FLAG_HT3,
  },
  
  { GPIOC,
    GPIO_Pin_4 | GPIO_Pin_5,
    GPIO_PinSource4,
    GPIO_PinSource5,
    USART1,
    DMA1_Channel4,
    DMA1_Channel5,
    DMA1_FLAG_TC4,
    DMA1_FLAG_TC5,
    DMA1_FLAG_HT5,
  }
}; 
  
void SerialLink::Init(
    SerialLinkDirection direction,
    uint32_t baud_rate,
    uint8_t* rx_buffer,
    size_t rx_block_size) {
  direction_ = direction;
  rx_buffer_ = rx_buffer;
  rx_block_size_ = rx_block_size;
  
  // Initialize clocks.
  if (direction == SERIAL_LINK_DIRECTION_LEFT) {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  } else {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  }
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  const DirectionDefinition& definition = direction_definition[direction];
  
  // Initialize GPIO.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
  gpio_init.GPIO_Pin = definition.tx_rx_pins;
  
  GPIO_Init(definition.gpio, &gpio_init);
  GPIO_PinAFConfig(definition.gpio, definition.tx_pin_source, GPIO_AF_7);
  GPIO_PinAFConfig(definition.gpio, definition.rx_pin_source, GPIO_AF_7);
  
  // Initialize USART.
  USART_InitTypeDef usart_init;
  usart_init.USART_BaudRate = baud_rate;
  usart_init.USART_WordLength = USART_WordLength_8b;
  usart_init.USART_StopBits = USART_StopBits_1;
  usart_init.USART_Parity = USART_Parity_No;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

  USART_Init(definition.usart, &usart_init);
  USART_Cmd(definition.usart, ENABLE);
  
  // Initialize DMA channels.
  DMA_InitTypeDef dma_init;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dma_init.DMA_Mode = DMA_Mode_Normal;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_M2M = DMA_M2M_Disable;
  dma_init.DMA_BufferSize = 0;
  dma_init.DMA_MemoryBaseAddr = 0;
  
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(definition.usart->TDR);
  dma_init.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_Cmd(definition.tx_dma_channel, DISABLE);
  DMA_DeInit(definition.tx_dma_channel);
  DMA_Init(definition.tx_dma_channel, &dma_init);
  USART_DMACmd(definition.usart, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(definition.usart->RDR);
  dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
  dma_init.DMA_Mode = rx_block_size_ == 0
      ? DMA_Mode_Normal
      : DMA_Mode_Circular;
  dma_init.DMA_MemoryBaseAddr = (uint32_t)rx_buffer_;
  dma_init.DMA_BufferSize = rx_block_size_ * 2;

  DMA_Cmd(definition.rx_dma_channel, DISABLE);
  DMA_DeInit(definition.rx_dma_channel);
  DMA_Init(definition.rx_dma_channel, &dma_init);
  
  if (rx_block_size_) {
    DMA_Cmd(definition.rx_dma_channel, ENABLE);
  }
}

void SerialLink::Transmit(const void* buffer, size_t size) {
  const DirectionDefinition& definition = direction_definition[direction_];

  // Disable DMA channel.
  definition.tx_dma_channel->CCR &= (uint16_t)(~DMA_CCR_EN);
  
  // Update size and buffer pointer.
  definition.tx_dma_channel->CNDTR = size;
  definition.tx_dma_channel->CMAR = (uint32_t)(buffer);

  // Clear DMA and USART TC flag.
  // DMA1->IFCR = definition.tx_complete_flag;
  // definition.usart->ICR = USART_FLAG_TC;
  
  // Enable DMA channel.
  definition.tx_dma_channel->CCR |= DMA_CCR_EN;
}

bool SerialLink::tx_complete() {
  const DirectionDefinition& definition = direction_definition[direction_];
  return DMA1->ISR & definition.tx_complete_flag;
}

void SerialLink::Receive(void* buffer, size_t size) {
  const DirectionDefinition& definition = direction_definition[direction_];
  
  // Disable DMA channel.
  definition.rx_dma_channel->CCR &= (uint16_t)(~DMA_CCR_EN);

  // Update size and buffer pointer.
  definition.rx_dma_channel->CNDTR = size;
  definition.rx_dma_channel->CMAR = (uint32_t)(buffer);

  // Clear DMA and USART RXNE flag.
  // DMA1->IFCR = definition.rx_complete_flag;
  // definition.usart->ICR = USART_FLAG_RXNE;

  // Enable DMA channel.
  definition.rx_dma_channel->CCR |= DMA_CCR_EN;
}

bool SerialLink::rx_complete() {
  const DirectionDefinition& definition = direction_definition[direction_];
  return DMA1->ISR & definition.rx_complete_flag;
}

const uint8_t* SerialLink::available_rx_buffer() {
  const DirectionDefinition& definition = direction_definition[direction_];
  uint32_t status = DMA1->ISR;
  if (status & definition.rx_half_complete_flag) {
    DMA1->IFCR = definition.rx_half_complete_flag;
    return &rx_buffer_[0];
  } else if (status & definition.rx_complete_flag) {
    DMA1->IFCR = definition.rx_complete_flag;
    return &rx_buffer_[rx_block_size_];
  } else {
    return NULL;
  }
}

}  // namespace stages
